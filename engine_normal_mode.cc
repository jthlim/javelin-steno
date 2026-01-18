//---------------------------------------------------------------------------

#include "clamp.h"
#include "console.h"
#include "engine.h"
#include "key.h"
#include "processor/paper_tape.h"
#include "segment.h"
#include "state.h"
#include "str.h"
#include "thread.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

#define ENABLE_PROFILE 0
#define ENABLE_PROFILE_SUGGESTIONS 0
#define DEBUG_SEGMENTS 0
#define DEBUG_KEY_CODE_BUFFERS 0
#define DEBUG_KEY_CODE_BUFFER_ELEMENTS 0

#if ENABLE_PROFILE || ENABLE_PROFILE_SUGGESTIONS
#include "arm/systick.h"
#endif

//---------------------------------------------------------------------------

struct StenoEngine::ConvertTextData {
  ConvertTextData(StenoEngine *engine, StenoKeyCodeBuffer *keyCodeBuffer,
                  StenoSegmentList *segments, size_t startingOffset,
                  bool executeSideEffects)
      : engine(engine), keyCodeBuffer(keyCodeBuffer), segments(segments),
        startingOffset(startingOffset), executeSideEffects(executeSideEffects) {
  }

  StenoEngine *const engine;
  StenoKeyCodeBuffer *const keyCodeBuffer;
  StenoSegmentList *const segments;
  const size_t startingOffset;
  const bool executeSideEffects;

  void ConvertText() {
    engine->ConvertText(*keyCodeBuffer, *segments, startingOffset,
                        executeSideEffects);
  }

  static void ThreadEntryPoint(void *data) {
    ((ConvertTextData *)data)->ConvertText();
  }
};

//---------------------------------------------------------------------------

constexpr size_t MAX_EXTRA_STROKES = 4;

size_t StenoEngine::GetStartingStrokeForNormalModeProcessing() const {
  const size_t maximumConversionStrokes =
      GetDictionary().GetMaximumOutlineLength();
  if (history.GetCount() < maximumConversionStrokes) {
    return 0;
  }

  const size_t startingStrokeIndex =
      history.GetStartingStroke(maximumConversionStrokes);

  // Code below is to expand the history scope when finger spelling is used
  // in case a set_value or retro_transform command is used.

  size_t lastLookupIndex = history.GetIndexOfWordStart(history.GetCount() - 1);
  const StenoState &lastState = history[lastLookupIndex].state;
  if (!lastState.isSpace && !lastState.isHistoryExtending &&
      !lastState.requestsHistoryExtending) {
    // Not fingerspelling -- trace back extra prefixes and suffixes up to limit.
    const size_t limit = startingStrokeIndex > MAX_EXTRA_STROKES
                             ? startingStrokeIndex - MAX_EXTRA_STROKES
                             : 0;
    while (lastLookupIndex > limit) {
      const size_t previousStartIndex =
          history.GetIndexOfWordStart(lastLookupIndex - 1);
      if (!history[lastLookupIndex].state.isSuffix &&
          !history[previousStartIndex].state.joinNext) {
        break;
      }
      lastLookupIndex = previousStartIndex;
    }
  } else {
    // Fingerspelling -- trace back as far as one unit.
    size_t loops = 1;
    while (lastLookupIndex > 0 &&
           history[lastLookupIndex].state.requestsHistoryExtending) {
      ++loops;
      lastLookupIndex = history.GetIndexOfWordStart(lastLookupIndex - 1);
    }

    do {
      while (lastLookupIndex > 0) {
        const size_t previousStartIndex =
            history.GetIndexOfWordStart(lastLookupIndex - 1);
        if (!history[previousStartIndex].state.isHistoryExtending) {
          break;
        }
        lastLookupIndex = previousStartIndex;
      }

      while (lastLookupIndex > 0) {
        const size_t previousStartIndex =
            history.GetIndexOfWordStart(lastLookupIndex - 1);
        if (!history[previousStartIndex].state.isSpace) {
          break;
        }
        lastLookupIndex = previousStartIndex;
      }
    } while (--loops);
  }

  return lastLookupIndex < startingStrokeIndex ? lastLookupIndex
                                               : startingStrokeIndex;
}

size_t StenoEngine::GetStartingStrokeForNormalModeUndoProcessing(
    size_t undoCount) const {
  const size_t lastSegmentStartStrokeIndex =
      history.GetIndexOfWordStart(history.GetCount() - undoCount);

  const StenoState state = history[lastSegmentStartStrokeIndex].state;

  if (lastSegmentStartStrokeIndex == history.GetCount() - undoCount &&
      state.historyRequirements == SegmentHistoryRequirements::NONE &&
      state.lookupType != SegmentLookupType::HISTORY_MODIFIED) {
    return lastSegmentStartStrokeIndex;
  }

  const size_t maximumConversionStrokes =
      GetDictionary().GetMaximumOutlineLength() + MAX_EXTRA_STROKES;

  const size_t conversionStrokes =
      state.historyRequirements == SegmentHistoryRequirements::ALL ||
              state.lookupType == SegmentLookupType::HISTORY_MODIFIED
          ? maximumConversionStrokes + undoCount
          : undoCount;

  return history.GetStartingStrokeAfterUndo(conversionStrokes);
}

void StenoEngine::ProcessNormalModeStroke(StenoStroke stroke) {
#if ENABLE_DICTIONARY_STATS
  StenoDictionary::ResetStats();
#endif

  history.PruneIfFull();

#if ENABLE_PROFILE
  sysTick->EnableCycleCount();
  const uint32_t t0 = sysTick->ReadCycleCount();
#endif

  const size_t previousSourceStrokeCount = history.GetCount();
  const size_t startingStroke = GetStartingStrokeForNormalModeProcessing();

  history.Add(stroke, state);

  const size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList nextSegments(conversionCount);
  BuildSegmentContext nextContext(nextSegments, *this);
  CreateSegments(nextContext, history.GetCount(), nextConversionBuffer,
                 conversionCount, nextSegments);

#if ENABLE_PROFILE
  const uint32_t t1 = sysTick->ReadCycleCount();
#endif

  StenoSegmentList previousSegments(conversionCount - 1);
  BuildSegmentContext previousContext(previousSegments, *this);
  if (nextConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(previousContext, previousSourceStrokeCount,
                   previousConversionBuffer, conversionCount - 1,
                   previousSegments);
  } else {
    CreateSegmentsUsingLongerResult(previousContext, previousSourceStrokeCount,
                                    previousConversionBuffer,
                                    conversionCount - 1, previousSegments,
                                    nextConversionBuffer, nextSegments);
  }
  if (nextContext.setValueText) {
    SetTemplateValue(nextContext.setValueIndex, nextContext.setValueText);
    nextContext.setValueText = nullptr;
  }

  // Update definition boundaries after evaluating previous segments so that
  // they are evaluated with the right lookup type.
  history.UpdateDefinitionBoundaries(
      history.GetCount() - conversionCount, nextSegments,
      nextConversionBuffer.segmentBuilder.GetStrokes(0));

#if ENABLE_PROFILE
  const uint32_t t2 = sysTick->ReadCycleCount();
#endif

  const size_t startingOffset =
      StenoSegmentList::GetCommonStartingSegmentsCount(previousSegments,
                                                       nextSegments);

#if DEBUG_SEGMENTS
  Console::Printf(
      "Previous segments:%s\n",
      previousConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()
          ? " (modified stroke history)"
          : "");
  for (size_t i = 0; i < previousSegments.GetCount(); ++i) {
    const StenoSegment &segment = previousSegments[i];
    const size_t strokeIndex = segment.GetStrokeIndex(
        previousConversionBuffer.segmentBuilder.GetStatePointer(0));
    const StenoStroke *strokes =
        previousConversionBuffer.segmentBuilder.GetStrokes(strokeIndex);
    Console::Printf(
        " %s%zu (%s): %T | \"%J\" | %s\n", i >= startingOffset ? "*" : "", i,
        segment.state->GetLookupTypeName(), strokes, segment.strokeLength,
        segment.lookup.GetText(), segment.state->GetHistoryRequirementsName());
  }

  Console::Printf("Next segments:%s\n",
                  nextConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()
                      ? " (modified stroke history)"
                      : "");
  for (size_t i = 0; i < nextSegments.GetCount(); ++i) {
    const StenoSegment &segment = nextSegments[i];
    const size_t strokeIndex = segment.GetStrokeIndex(
        nextConversionBuffer.segmentBuilder.GetStatePointer(0));
    const StenoStroke *strokes =
        nextConversionBuffer.segmentBuilder.GetStrokes(strokeIndex);
    Console::Printf(
        " %s%zu (%s): %T | \"%J\" | %s\n", i >= startingOffset ? "*" : "", i,
        segment.state->GetLookupTypeName(), strokes, segment.strokeLength,
        segment.lookup.GetText(), segment.state->GetHistoryRequirementsName());
  }
  Console::Printf("\n");
#endif

#if ENABLE_PROFILE
  const uint32_t t3 = sysTick->ReadCycleCount();
#endif

  ConvertTextData previousConvertTextData(
      this, &previousConversionBuffer.keyCodeBuffer, &previousSegments,
      startingOffset, false);
#if !JAVELIN_THREADS
  previousConvertTextData.ConvertText();
#endif
  ConvertTextData nextConvertTextData(this, &nextConversionBuffer.keyCodeBuffer,
                                      &nextSegments, startingOffset, true);
#if JAVELIN_THREADS
  RunParallel(&ConvertTextData::ThreadEntryPoint, &previousConvertTextData,
              &ConvertTextData::ThreadEntryPoint, &nextConvertTextData);
#else
  nextConvertTextData.ConvertText();
#endif

#if ENABLE_PROFILE
  const uint32_t t4 = sysTick->ReadCycleCount();
#endif

#if DEBUG_KEY_CODE_BUFFERS
  {
    char *previousText =
        previousConversionBuffer.keyCodeBuffer.ToUnresolvedString();
    Console::Printf("PreviousText: \"%J\"\n", previousText);
    free(previousText);

    char *nextText = nextConversionBuffer.keyCodeBuffer.ToUnresolvedString();
    Console::Printf("NextText: \"%J\"\n\n", nextText);
    free(nextText);
  }
#endif

#if DEBUG_KEY_CODE_BUFFER_ELEMENTS
  DumpKeyCodeBufferElements();
#endif

  state = nextConversionBuffer.keyCodeBuffer.GetPersistentState();

  bool printSuggestions = true;
  const bool canCombine =
      emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer);

#if ENABLE_PROFILE
  const uint32_t t5 = sysTick->ReadCycleCount();
#endif

  if (canCombine && previousSegments.GetCount() < nextSegments.GetCount()) {
    history.SetBackCombineUndo();

    if (previousConversionBuffer.keyCodeBuffer.GetCount() ==
        nextConversionBuffer.keyCodeBuffer.GetCount()) {
      history.SetBackHasManualStateChange();
      if (state.caseMode != StenoCaseMode::NORMAL ||
          state.overrideCaseMode != StenoCaseMode::NORMAL) {
        printSuggestions = false;
      } else if (history.GetCount() >= 2) {
        const StenoState previousState = history.Back(2).state;
        if (previousState.isManualStateChange) {
          printSuggestions = false;
        }
      }
    }
  }

  PrintTextLog(previousConversionBuffer.keyCodeBuffer,
               nextConversionBuffer.keyCodeBuffer);
  PrintPaperTape(stroke, previousSegments, nextSegments);

  if (nextConversionBuffer.keyCodeBuffer.launchAddTranslation) {
    history.SetBackNoCombineUndo();
    InitiateAddTranslationMode(
        nextConversionBuffer.keyCodeBuffer.addTranslationText);
    return;
  }

  if (nextConversionBuffer.keyCodeBuffer.launchConsole) {
    history.SetBackNoCombineUndo();
    InitiateConsoleMode();
    return;
  }

  if (nextConversionBuffer.keyCodeBuffer.doResetState) {
    ResetState();
    return;
  }

#if ENABLE_PROFILE
  const uint32_t t6 = sysTick->ReadCycleCount();
#endif

  if (printSuggestions) {
    // PrintSuggestions will overwrite the previousConversionBuffer
    PrintSuggestions(previousSegments, nextSegments);
  }

#if ENABLE_PROFILE
  const uint32_t t7 = sysTick->ReadCycleCount();
#endif

#if ENABLE_PROFILE
  Console::Printf("Next Segments: %u\n", t1 - t0);
  Console::Printf("Previous Segments: %u\n", t2 - t1);
  Console::Printf("Common Check: %u\n", t3 - t2);
  Console::Printf("Text Conversion: %u\n", t4 - t3);
  Console::Printf("Text Emitter: %u\n", t5 - t4);
  Console::Printf("Other handling: %u\n", t6 - t5);
  Console::Printf("Suggestions: %u\n", t7 - t6);
#endif

#if ENABLE_DICTIONARY_STATS
  Console::Printf("Lookups: %zu\n", StenoDictionary::GetLookupCount());
  Console::Printf("ReverseLookups: %zu\n",
                  StenoDictionary::GetReverseLookupCount());
  Console::Printf("DictionaryForOutline: %zu\n",
                  StenoDictionary::GetDictionaryForOutlineCount());
#endif

#if ENABLE_PROFILE || ENABLE_DICTIONARY_STATS
  Console::Printf("\n");
#endif
}

void StenoEngine::ProcessNormalModeUndo() {
#if ENABLE_PROFILE
  const uint32_t t0 = sysTick->ReadCycleCount();
#endif

  const size_t undoCount = history.GetUndoCount();
  if (undoCount == 0) {
    Key::Tap(KeyCode::BACKSPACE);
    PrintPaperTapeUndo(0);
    return;
  }

  const size_t startingStroke =
      GetStartingStrokeForNormalModeUndoProcessing(undoCount);
  const size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList previousSegments(conversionCount);
  BuildSegmentContext previousContext(previousSegments, *this);
  CreateSegments(previousContext, history.GetCount(), previousConversionBuffer,
                 conversionCount, previousSegments);

#if ENABLE_PROFILE
  const uint32_t t1 = sysTick->ReadCycleCount();
#endif

  state = history.Back(undoCount).state.GetPersistentState();
  history.RemoveBack(undoCount);

  const size_t nextConversionCount =
      undoCount >= conversionCount ? 0 : conversionCount - undoCount;

  // Mark last lookup type as unknown to ensure a full lookup is performed.
  // Without this, undoing 'obliterates' does nto work as expected.
  const StenoState backState = history.Back().state;
  history.MarkLastLookupTypeAsUnknown();

  StenoSegmentList nextSegments(nextConversionCount);
  BuildSegmentContext nextContext(nextSegments, *this);
  if (previousConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(nextContext, history.GetCount(), nextConversionBuffer,
                   nextConversionCount, nextSegments);

  } else {
    CreateSegmentsUsingLongerResult(nextContext, history.GetCount(),
                                    nextConversionBuffer, nextConversionCount,
                                    nextSegments, previousConversionBuffer,
                                    previousSegments);
  }
  if (nextContext.setValueText) {
    SetTemplateValue(nextContext.setValueIndex, nextContext.setValueText);
    nextContext.setValueText = nullptr;
  }

  history.Back().state = backState;

#if ENABLE_PROFILE
  const uint32_t t2 = sysTick->ReadCycleCount();
#endif

  history.UpdateDefinitionBoundaries(
      history.GetCount() - nextConversionCount, nextSegments,
      nextConversionBuffer.segmentBuilder.GetStrokes(0));

  const size_t startingOffset =
      StenoSegmentList::GetCommonStartingSegmentsCount(previousSegments,
                                                       nextSegments);

#if DEBUG_SEGMENTS
  Console::Printf(
      "Previous segments:%s\n",
      previousConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()
          ? " (modified stroke history)"
          : "");
  for (size_t i = 0; i < previousSegments.GetCount(); ++i) {
    const StenoSegment &segment = previousSegments[i];
    const size_t strokeIndex = segment.GetStrokeIndex(
        previousConversionBuffer.segmentBuilder.GetStatePointer(0));
    const StenoStroke *strokes =
        previousConversionBuffer.segmentBuilder.GetStrokes(strokeIndex);
    Console::Printf(
        " %s%zu (%s): %T | \"%J\" | %s\n", i >= startingOffset ? "*" : "", i,
        segment.state->GetLookupTypeName(), strokes, segment.strokeLength,
        segment.lookup.GetText(), segment.state->GetHistoryRequirementsName());
  }

  Console::Printf("Next segments:%s\n",
                  nextConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()
                      ? " (modified stroke history)"
                      : "");
  for (size_t i = 0; i < nextSegments.GetCount(); ++i) {
    const StenoSegment &segment = nextSegments[i];
    const size_t strokeIndex = segment.GetStrokeIndex(
        nextConversionBuffer.segmentBuilder.GetStatePointer(0));
    const StenoStroke *strokes =
        nextConversionBuffer.segmentBuilder.GetStrokes(strokeIndex);
    Console::Printf(
        " %s%zu (%s): %T | \"%J\" | %s\n", i >= startingOffset ? "*" : "", i,
        segment.state->GetLookupTypeName(), strokes, segment.strokeLength,
        segment.lookup.GetText(), segment.state->GetHistoryRequirementsName());
  }
  Console::Printf("\n");
#endif

#if ENABLE_PROFILE
  const uint32_t t3 = sysTick->ReadCycleCount();
#endif

  ConvertTextData previousConvertTextData(
      this, &previousConversionBuffer.keyCodeBuffer, &previousSegments,
      startingOffset, false);
#if !JAVELIN_THREADS
  previousConvertTextData.ConvertText();
#endif

  ConvertTextData nextConvertTextData(this, &nextConversionBuffer.keyCodeBuffer,
                                      &nextSegments, startingOffset, false);

#if !JAVELIN_THREADS
  nextConvertTextData.ConvertText();
#else
  RunParallel(&ConvertTextData::ThreadEntryPoint, &previousConvertTextData,
              &ConvertTextData::ThreadEntryPoint, &nextConvertTextData);
#endif

#if ENABLE_PROFILE
  const uint32_t t4 = sysTick->ReadCycleCount();
#endif

#if DEBUG_KEY_CODE_BUFFERS
  {
    char *previousText =
        previousConversionBuffer.keyCodeBuffer.ToUnresolvedString();
    Console::Printf("PreviousText: \"%J\"\n", previousText);
    free(previousText);

    char *nextText = nextConversionBuffer.keyCodeBuffer.ToUnresolvedString();
    Console::Printf("NextText: \"%J\"\n\n", nextText);
    free(nextText);
  }
#endif

#if DEBUG_KEY_CODE_BUFFER_ELEMENTS
  DumpKeyCodeBufferElements();
#endif

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

#if ENABLE_PROFILE
  const uint32_t t5 = sysTick->ReadCycleCount();
#endif

  PrintTextLog(previousConversionBuffer.keyCodeBuffer,
               nextConversionBuffer.keyCodeBuffer);
  PrintPaperTapeUndo(undoCount);

#if ENABLE_PROFILE
  Console::Printf("Previous Segments: %u\n", t1 - t0);
  Console::Printf("Next Segments: %u\n", t2 - t1);
  Console::Printf("Common Check: %u\n", t3 - t2);
  Console::Printf("Text Conversion: %u\n", t4 - t3);
  Console::Printf("Text Emitter: %u\n\n", t5 - t4);
#endif
}

void StenoEngine::CreateSegments(BuildSegmentContext &context,
                                 size_t sourceStrokeCount,
                                 ConversionBuffer &buffer,
                                 size_t conversionLimit,
                                 StenoSegmentList &segments) {
  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);
  buffer.segmentBuilder.CreateSegments(context);
}

void StenoEngine::CreateSegmentsUsingLongerResult(
    BuildSegmentContext &context, size_t sourceStrokeCount,
    ConversionBuffer &buffer, size_t conversionLimit,
    StenoSegmentList &segments, const ConversionBuffer &longerBuffer,
    const StenoSegmentList &longerSegments) {

  // state pointers are used to track segment length, so a proper stroke
  // history needs to be set up and translated.

  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);

  size_t startingOffset = 0;
  if (conversionLimit > sourceStrokeCount) {
    conversionLimit = sourceStrokeCount;
  }
  for (const StenoSegment &segment : longerSegments) {
    if (startingOffset + segment.strokeLength > conversionLimit) {
      break;
    }
    startingOffset += segment.strokeLength;
    segments.Add(StenoSegment(
        segment.strokeLength, segment.lookupType,
        buffer.segmentBuilder.GetStatePointer(
            longerBuffer.segmentBuilder.GetStateIndex(segment.state)),
        segment.lookup.Clone()));
  }

  if (startingOffset == conversionLimit) {
    return;
  }

  buffer.segmentBuilder.TransferStartFrom(longerBuffer.segmentBuilder,
                                          startingOffset);
  buffer.segmentBuilder.CreateSegments(context, startingOffset);
}

void StenoEngine::ConvertText(StenoKeyCodeBuffer &keyCodeBuffer,
                              StenoSegmentList &segments, size_t startingOffset,
                              bool executeSideEffects) {
  StenoState endState;
  if (startingOffset == segments.GetCount()) {
    keyCodeBuffer.Reset();
    // The call sites of ConvertText ensure that the StenoEngine's state here is
    // equivalent to what it would be if segments had been converted up to the
    // starting offset.
    endState = state;
  } else {
    StenoTokenizer tokenizer(segments, startingOffset);
    keyCodeBuffer.Populate(tokenizer, executeSideEffects);
    endState = keyCodeBuffer.state;
  }

  if (placeSpaceAfter && !endState.joinNext) {
    keyCodeBuffer.AppendSpace();
  }
}

#if DEBUG_KEY_CODE_BUFFER_ELEMENTS

void StenoEngine::DumpKeyCodeBufferElements() const {
  Console::Printf("Previous: %zu\n",
                  previousConversionBuffer.keyCodeBuffer.GetCount());
  for (size_t i = 0; i < previousConversionBuffer.keyCodeBuffer.GetCount();
       ++i) {
    StenoKeyCode keyCode = previousConversionBuffer.keyCodeBuffer.buffer[i];
    if (keyCode.IsRawKeyCode()) {
      Console::Printf("%zu: raw %u: %d\n", i, keyCode.GetRawKeyCode(),
                      keyCode.IsPress());
    } else {
      Console::Printf("%zu: '%C'\n", i, keyCode.GetUnicode());
    }
  }

  Console::Printf("Next: %zu\n", nextConversionBuffer.keyCodeBuffer.GetCount());
  for (size_t i = 0; i < nextConversionBuffer.keyCodeBuffer.GetCount(); ++i) {
    StenoKeyCode keyCode = nextConversionBuffer.keyCodeBuffer.buffer[i];
    if (keyCode.IsRawKeyCode()) {
      Console::Printf("%zu: raw %u: %d\n", i, keyCode.GetRawKeyCode(),
                      keyCode.IsPress());
    } else {
      Console::Printf("%zu: '%C'\n", i, keyCode.GetUnicode());
    }
  }
  Console::Printf("\n");
}

#endif

//---------------------------------------------------------------------------

void StenoEngine::PrintPaperTapeUndo(size_t undoCount) const {
  if (!Console::IsEventEnabled(ConsoleEvent::PAPER_TAPE)) {
    return;
  }

  Console::Printf("EV {e: p,o: \"%t\",u: %zu}\n\n", &undoStroke, undoCount);
}

void StenoEngine::PrintPaperTape(StenoStroke stroke,
                                 const StenoSegmentList &previousSegments,
                                 const StenoSegmentList &nextSegments) const {
  if (!Console::IsEventEnabled(ConsoleEvent::PAPER_TAPE)) {
    return;
  }

  Console::Printf("EV {e: p,o: \"%t\"", &stroke);

  size_t undoCount = 0;
  const StenoSegment &segment = nextSegments.Back();
  const size_t startingStrokeIndex =
      segment.GetStrokeIndex(nextSegments.Front().state);

  const size_t previousSegmentCount = previousSegments.GetCount();
  while (undoCount < previousSegmentCount &&
         previousSegments[previousSegmentCount - 1 - undoCount].GetStrokeIndex(
             previousSegments.Front().state) >= startingStrokeIndex) {
    ++undoCount;
  }

  if (undoCount > 0) {
    Console::Printf(",u: %zu", undoCount);
  }

  const StenoStroke *strokes =
      nextConversionBuffer.segmentBuilder.GetStrokes(startingStrokeIndex);
  const size_t length = segment.strokeLength;
  const StenoDictionary *provider =
      GetDictionary().GetDictionaryForOutline(strokes, length);
  if (provider != nullptr) {
    Console::Printf(",d: %Y", provider->GetName());
  }

  // Unescape buffer commands.
  const char *lookup = segment.lookup.GetText();
  if (Str::HasPrefix(lookup, "{:=")) {
    const char *p = lookup + 3;
    BufferWriter writer;
    while (*p && *p != '}') {
      int c = *p++;
      if (c == '\\') {
        c = *p++;
      }
      writer.WriteByte(c);
    }
    writer.WriteByte('\0');
    Console::Printf(",t: %Y}\n\n", writer.GetBuffer());
  } else {
    Console::Printf(",t: %Y}\n\n", lookup);
  }
}

void StenoEngine::PrintSuggestions(const StenoSegmentList &previousSegments,
                                   const StenoSegmentList &nextSegments) {
#if ENABLE_PROFILE_SUGGESTIONS
  sysTick->EnableCycleCount();
  const uint32_t t0 = sysTick->ReadCycleCount();
#endif

  if (!Console::IsEventEnabled(ConsoleEvent::SUGGESTION)) {
    return;
  }

  // Finger spelling suggestions.
  if (Str::IsFingerSpellingCommand(nextSegments.Back().lookup.GetText())) {
    if (state.isManualStateChange || state.joinNext) {
      return;
    }

    PrintFingerSpellingSuggestions(previousSegments, nextSegments);
    return;
  }

  // General suggestions. Search back up to 8 word segments.
  size_t startSegmentIndex = nextSegments.GetCount();
  if (startSegmentIndex == 0) {
    return;
  }

#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t1 = sysTick->ReadCycleCount();
  Console::Printf("PrintSuggestions t01: %u\n\n", t1 - t0);
#endif

  char *lastLookup = nullptr;
  size_t strokeThresholdCount = 0;
  for (size_t segmentCount = 1; segmentCount < 8; ++segmentCount) {
#if ENABLE_PROFILE_SUGGESTIONS
    const uint32_t t20 = sysTick->ReadCycleCount();
#endif
    Pump();

#if ENABLE_PROFILE_SUGGESTIONS
    const uint32_t t21 = sysTick->ReadCycleCount();
#endif
    char *newLookup;

    for (;;) {
      if (startSegmentIndex == 0) {
        goto exit;
      }
      --startSegmentIndex;

      if (nextSegments.GetCount() - startSegmentIndex >=
          PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT) {
        goto exit;
      }

      const StenoSegment &segment = nextSegments[startSegmentIndex];
      if (segment.ContainsKeyCode() ||
          segment.lookup == StenoDictionaryLookupResult::NO_OP) {
        goto exit;
      }
      strokeThresholdCount += segment.strokeLength;

      // Consider it a segment start if it isn't a suffix stroke and it isn't
      // a fingerspelling joined to a previous fingerspelling.
      // This will still give suggestions after prefixes, e.g.
      //   overwatching: AUFR/WAFP/-G will suggest to combine WAFPG
      const char *startSegmentText = segment.lookup.GetText();

      if (!Str::IsJoinPrevious(startSegmentText) &&
          (startSegmentIndex == 0 ||
           !Str::IsFingerSpellingCommand(startSegmentText) ||
           !Str::IsFingerSpellingCommand(
               nextSegments[startSegmentIndex - 1].lookup.GetText()))) {
        break;
      }
    }

#if ENABLE_PROFILE_SUGGESTIONS
    const uint32_t t22 = sysTick->ReadCycleCount();
#endif

    newLookup = PrintSegmentSuggestion(startSegmentIndex, strokeThresholdCount,
                                       nextSegments, lastLookup);

#if ENABLE_PROFILE_SUGGESTIONS
    const uint32_t t23 = sysTick->ReadCycleCount();
#endif
    free(lastLookup);
    lastLookup = newLookup;

#if ENABLE_PROFILE_SUGGESTIONS
    const uint32_t t24 = sysTick->ReadCycleCount();
    Console::Printf("PrintSuggestions %zu: %u, %u, %u, %u\n\n", segmentCount,
                    t21 - t20, t22 - t21, t23 - t22, t24 - t23);
#endif

    if (!lastLookup) {
      return;
    }
  }
exit:
  free(lastLookup);
}

void StenoEngine::PrintFingerSpellingSuggestions(
    const StenoSegmentList &previousSegments,
    const StenoSegmentList &nextSegments) {
  // Get the last word out of the buffer and look that up.
  char buffer[256];
  char *p = buffer + sizeof(buffer) - 2;
  const StenoKeyCode *skc =
      nextConversionBuffer.keyCodeBuffer.currentOutput - 1;

  if (placeSpaceAfter && skc > nextConversionBuffer.keyCodeBuffer.buffer) {
    skc--;
  }

  size_t keyCodeCount = 0;
  while (skc >= nextConversionBuffer.keyCodeBuffer.buffer &&
         !skc->IsWhitespace() && !skc->IsRawKeyCode()) {
    const uint32_t unicode = skc->GetUnicode();
    const size_t length = Utf8Pointer::BytesForCharacterCode(unicode);
    p -= length;
    if (p <= buffer) {
      return;
    }
    Utf8Pointer(p).SetAndAdvance(unicode);
    ++keyCodeCount;
    --skc;
  }

  if (keyCodeCount >= 3) {
    char t = buffer[253];
    buffer[253] = '^';
    buffer[254] = '}';
    buffer[255] = '\0';
    p[-1] = '{';
    PrintSuggestion(p - 1, 1, keyCodeCount - 1);
    buffer[253] = t;
  }

  if (keyCodeCount >= 2) {
    buffer[254] = '\0';
    PrintSuggestion(p, 1, keyCodeCount);
  }
}

void StenoEngine::PrintSuggestion(const char *p, size_t arrowPrefixCount,
                                  size_t strokeThreshold) const {
  StenoReverseDictionaryLookup lookup(p, strokeThreshold);
  ReverseLookup(lookup);
  if (lookup.results.IsEmpty()) {
    return;
  }

  Console::Printf("EV {e: s,c: %zu,t: %Y,o: [", arrowPrefixCount, p);
  for (size_t i = 0; i < lookup.results.GetCount(); ++i) {
    const StenoReverseDictionaryResult &entry = lookup.results[i];
    Console::Printf(i == 0 ? "\"%T\"" : ",\"%T\"", entry.strokes, entry.length);
  }
  Console::Printf("]");
  if (lookup.AreAllResultsFromSameDictionary()) {
    const StenoDictionary *dictionary = lookup.results.Front().dictionary;
    if (!dictionary->IsInternal()) {
      Console::Printf(",d: %Y", dictionary->GetName());
    }
  }
  Console::Printf("}\n\n");
}

static bool ShouldShowSuggestions(const StenoSegmentList &segments,
                                  size_t startSegmentIndex) {
  // Count the number of suffix "{*!}" entries.
  // There must be more that number of entries before that.
  size_t joinPreviousCount = 0;
  for (size_t i = segments.GetCount(); i > startSegmentIndex;) {
    --i;
    if (!Str::Eq(segments[i].lookup.GetText(), "{*!}")) {
      break;
    }
    ++joinPreviousCount;
  }
  return (segments.GetCount() - startSegmentIndex) > 2 * joinPreviousCount;
}

char *StenoEngine::PrintSegmentSuggestion(size_t startSegmentIndex,
                                          size_t strokeThresholdCount,
                                          const StenoSegmentList &segments,
                                          char *lastLookup) {
#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t0 = sysTick->ReadCycleCount();
#endif

  if (!ShouldShowSuggestions(segments, startSegmentIndex)) {
    return Str::CreateEmpty();
  }

#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t1 = sysTick->ReadCycleCount();
#endif

  StenoTokenizer tokenizer(segments, startSegmentIndex);
  previousConversionBuffer.keyCodeBuffer.Populate(tokenizer, false);

#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t2 = sysTick->ReadCycleCount();
#endif

  char *lookup =
      segments.HasManualStateChange(startSegmentIndex) ||
              Str::HasPrefix(segments.Back().lookup.GetText(), "{:")
          ? previousConversionBuffer.keyCodeBuffer.ToString()
          : previousConversionBuffer.keyCodeBuffer.ToUnresolvedString();

  // Special case {*!} and function calls at the start to avoid suggestions.

  if (const char *firstSegmentText =
          segments[startSegmentIndex].lookup.GetText();
      firstSegmentText[0] == '{' &&
      (firstSegmentText[1] == ':' || // HasPrefix("{:")
       Str::Eq(firstSegmentText, "{*!}"))) {
    return lookup;
  }

  char *spaceRemoved = *lookup == ' ' ? lookup + 1 : lookup;
  bool printSuggestion =
      *spaceRemoved != '\0' && (startSegmentIndex != segments.GetCount() - 1 ||
                                strokeThresholdCount != 1);

#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t3 = sysTick->ReadCycleCount();
#endif

  if (state.joinNext) {
    bool usePrefixSyntax = true;
    if (segments.GetCount() >= 2) {
      // If the new state ends in a space, truncate the space and treat it
      // as a non-space lookup.
      const size_t length = Str::Length(spaceRemoved);
      if (length != 0 && spaceRemoved[length - 1] == ' ') {
        spaceRemoved = Str::DupN(spaceRemoved, length - 1);
        free(lookup);
        lookup = spaceRemoved;
        usePrefixSyntax = false;
      }
    }

    if (usePrefixSyntax) {
      char *prefixLookup = Str::Asprintf("{%s^}", spaceRemoved);
      free(lookup);
      lookup = prefixLookup;
      spaceRemoved = prefixLookup;
    }
  }

  if (!printSuggestion && lastLookup) {
    char *lastLookupSpaceRemoved =
        *lastLookup == ' ' ? lastLookup + 1 : lastLookup;

    printSuggestion = !Str::Eq(spaceRemoved, lastLookupSpaceRemoved);
  }

#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t4 = sysTick->ReadCycleCount();
#endif

  if (printSuggestion) {
    PrintSuggestion(spaceRemoved, strokeThresholdCount, strokeThresholdCount);
  }

#if ENABLE_PROFILE_SUGGESTIONS
  const uint32_t t5 = sysTick->ReadCycleCount();
  Console::Printf("PrintSegmentSuggestions: %u, %u, %u, %u, %u\n\n", t1 - t0,
                  t2 - t1, t3 - t2, t4 - t3, t5 - t4);
#endif

  return lookup;
}

void StenoEngine::PrintTextLog(
    const StenoKeyCodeBuffer &previousKeyCodeBuffer,
    const StenoKeyCodeBuffer &nextKeyCodeBuffer) const {
  if (!Console::IsEventEnabled(ConsoleEvent::TEXT)) {
    return;
  }

  const StenoKeyCode *previousData = previousKeyCodeBuffer.buffer;
  const size_t previousLength = previousKeyCodeBuffer.GetCount();
  const StenoKeyCode *nextData = nextKeyCodeBuffer.buffer;
  const size_t nextLength = nextKeyCodeBuffer.GetCount();
  size_t i = 0;
  while (i < previousLength && i < nextLength &&
         previousData[i].HasSameOutput(nextData[i])) {
    ++i;
  }

  size_t backspaceCount = 0;
  for (size_t j = i; j < previousLength; ++j) {
    if (!previousData[j].IsRawKeyCode()) {
      ++backspaceCount;
    }
  }

  Console::Printf("EV {e: t,t: \"");
  static constexpr char BACKSPACES[] =
      "\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b";

  while (backspaceCount > 0) {
    const size_t writeCount = ClampMax(backspaceCount, 16);
    Console::Write(BACKSPACES, 2 * writeCount);
    backspaceCount -= writeCount;
  }

  if (i < nextLength) {
    char *text = nextKeyCodeBuffer.ToString(i);
    Console::WriteAsJson(text);
    free(text);
  }
  Console::Printf("\"}\n\n");
}

//---------------------------------------------------------------------------

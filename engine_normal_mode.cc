//---------------------------------------------------------------------------

#include "clock.h"
#include "console.h"
#include "engine.h"
#include "key.h"
#include "segment.h"
#include "state.h"
#include "str.h"
#include "thread.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

#define ENABLE_PROFILE 0

//---------------------------------------------------------------------------

#if JAVELIN_THREADS

struct StenoEngine::UpdateNormalModeTextBufferThreadData {
  UpdateNormalModeTextBufferThreadData(StenoEngine *engine,
                                       ConversionBuffer *conversionBuffer,
                                       StenoSegmentList *segmentList,
                                       size_t startingOffset)
      : engine(engine), conversionBuffer(conversionBuffer),
        segmentList(segmentList), startingOffset(startingOffset) {}

  StenoEngine *engine;
  ConversionBuffer *conversionBuffer;
  StenoSegmentList *segmentList;
  size_t startingOffset;

  void ConvertText() {
    engine->ConvertText(*conversionBuffer, *segmentList, startingOffset);
  }

  static void ConvertTextEntryPoint(void *data) {
    ((UpdateNormalModeTextBufferThreadData *)data)->ConvertText();
  }
};

#endif

void StenoEngine::ProcessNormalModeStroke(StenoStroke stroke) {
#if ENABLE_DICTIONARY_STATS
  StenoDictionary::ResetStats();
#endif

  history.PruneIfFull();

  const size_t previousSourceStrokeCount = history.GetCount();
  history.Add(stroke, state);

#if ENABLE_PROFILE
  uint32_t t0 = Clock::GetMicroseconds();
#endif

  const size_t maximumConversionStrokes =
      dictionary.GetMaximumOutlineLength() +
      SEGMENT_CONVERSION_PREFIX_SUFFIX_LIMIT;
  const size_t startingStroke =
      history.GetStartingStroke(maximumConversionStrokes);
  const size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList nextSegmentList;
  CreateSegments(history.GetCount(), nextConversionBuffer, conversionCount,
                 nextSegmentList);
  history.UpdateDefinitionBoundaries(history.GetCount() - conversionCount,
                                     nextSegmentList);

#if ENABLE_PROFILE
  uint32_t t1 = Clock::GetMicroseconds();
#endif

  StenoSegmentList previousSegmentList;
  if (nextConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(previousSourceStrokeCount, previousConversionBuffer,
                   conversionCount - 1, previousSegmentList);
  } else {
    CreateSegmentsUsingLongerResult(previousSourceStrokeCount,
                                    previousConversionBuffer,
                                    conversionCount - 1, previousSegmentList,
                                    nextConversionBuffer, nextSegmentList);
  }

#if ENABLE_PROFILE
  uint32_t t2 = Clock::GetMicroseconds();
#endif

  size_t startingOffset = StenoSegmentList::GetCommonStartingSegmentsCount(
      previousSegmentList, nextSegmentList);
  if (startingOffset > 0 && placeSpaceAfter) {
    --startingOffset;
  }

#if ENABLE_PROFILE
  uint32_t t3 = Clock::GetMicroseconds();
#endif

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData previousThreadData(
      this, &previousConversionBuffer, &previousSegmentList, startingOffset);
  UpdateNormalModeTextBufferThreadData nextThreadData(
      this, &nextConversionBuffer, &nextSegmentList, startingOffset);

  RunParallel(&UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &previousThreadData,
              &UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &nextThreadData);
#else
  ConvertText(previousConversionBuffer, previousSegmentList, startingOffset);
  ConvertText(nextConversionBuffer, nextSegmentList, startingOffset);
#endif

#if ENABLE_PROFILE
  uint32_t t4 = Clock::GetMicroseconds();
#endif

  state = nextConversionBuffer.keyCodeBuffer.state;
  state.shouldCombineUndo = false;
  state.isManualStateChange = false;

  bool printSuggestions = true;
  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    history.SetBackCombineUndo();

    if (previousConversionBuffer.keyCodeBuffer.count ==
        nextConversionBuffer.keyCodeBuffer.count) {
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
  PrintPaperTape(stroke, previousSegmentList, nextSegmentList);

  if (nextConversionBuffer.keyCodeBuffer.addTranslationCount >
      previousConversionBuffer.keyCodeBuffer.addTranslationCount) {
    history.SetBackNoCombineUndo();
    InitiateAddTranslationMode();
    return;
  }

  if (nextConversionBuffer.keyCodeBuffer.consoleCount >
      previousConversionBuffer.keyCodeBuffer.consoleCount) {
    history.SetBackNoCombineUndo();
    InitiateConsoleMode();
    return;
  }

  if (nextConversionBuffer.keyCodeBuffer.resetStateCount >
      previousConversionBuffer.keyCodeBuffer.resetStateCount) {
    ResetState();
    return;
  }

#if ENABLE_PROFILE
  uint32_t t5 = Clock::GetMicroseconds();
#endif

  if (printSuggestions) {
    // PrintSuggestions will overwrite the previousConversionBuffer
    PrintSuggestions(previousSegmentList, nextSegmentList);
  }

#if ENABLE_PROFILE
  uint32_t t6 = Clock::GetMicroseconds();
#endif

#if ENABLE_PROFILE
  Console::Printf("Next Segments: %u\n", t1 - t0);
  Console::Printf("Previous Segments: %u\n", t2 - t1);
  Console::Printf("Common Check: %u\n", t3 - t2);
  Console::Printf("Text Conversion: %u\n", t4 - t3);
  Console::Printf("Suggestions: %u\n", t6 - t5);
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
  const size_t maximumConversionStrokes =
      dictionary.GetMaximumOutlineLength() +
      SEGMENT_CONVERSION_PREFIX_SUFFIX_LIMIT;

  const size_t undoCount = history.GetUndoCount(maximumConversionStrokes);
  if (undoCount == 0) {
    Key::Press(KeyCode::BACKSPACE);
    Key::Release(KeyCode::BACKSPACE);
    PrintPaperTapeUndo(0);
    return;
  }

  const size_t startingStroke =
      history.GetStartingStroke(maximumConversionStrokes);
  const size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList previousSegmentList;
  CreateSegments(history.GetCount(), previousConversionBuffer, conversionCount,
                 previousSegmentList);

  state = history.Back(undoCount).state;
  state.shouldCombineUndo = false;
  state.isManualStateChange = false;
  history.RemoveBack(undoCount);

  const size_t nextConversionCount =
      undoCount >= conversionCount ? 0 : conversionCount - undoCount;

  StenoSegmentList nextSegmentList;
  if (previousConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(history.GetCount(), nextConversionBuffer,
                   nextConversionCount, nextSegmentList);

  } else {
    CreateSegmentsUsingLongerResult(
        history.GetCount(), nextConversionBuffer, nextConversionCount,
        nextSegmentList, previousConversionBuffer, previousSegmentList);
  }

  history.UpdateDefinitionBoundaries(history.GetCount() - nextConversionCount,
                                     nextSegmentList);

  size_t startingOffset = StenoSegmentList::GetCommonStartingSegmentsCount(
      previousSegmentList, nextSegmentList);
  if (startingOffset > 0 && placeSpaceAfter) {
    --startingOffset;
  }

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData previousThreadData(
      this, &previousConversionBuffer, &previousSegmentList, startingOffset);
  UpdateNormalModeTextBufferThreadData nextThreadData(
      this, &nextConversionBuffer, &nextSegmentList, startingOffset);

  RunParallel(&UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &previousThreadData,
              &UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &nextThreadData);
#else
  ConvertText(previousConversionBuffer, previousSegmentList, startingOffset);
  ConvertText(nextConversionBuffer, nextSegmentList, startingOffset);
#endif

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  PrintTextLog(previousConversionBuffer.keyCodeBuffer,
               nextConversionBuffer.keyCodeBuffer);
  PrintPaperTapeUndo(undoCount);
}

void StenoEngine::CreateSegments(size_t sourceStrokeCount,
                                 ConversionBuffer &buffer,
                                 size_t conversionLimit,
                                 StenoSegmentList &segmentList) {
  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);
  BuildSegmentContext context(segmentList, dictionary, orthography);
  buffer.segmentBuilder.CreateSegments(context);
}

void StenoEngine::CreateSegmentsUsingLongerResult(
    size_t sourceStrokeCount, ConversionBuffer &buffer, size_t conversionLimit,
    StenoSegmentList &segmentList, const ConversionBuffer &longerBuffer,
    const StenoSegmentList &longerSegmentList) {

  // state pointers are used to track segment length, so a proper stroke
  // history needs to be set up and translated.

  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);

  size_t startingOffset = 0;
  if (conversionLimit > sourceStrokeCount) {
    conversionLimit = sourceStrokeCount;
  }
  for (const StenoSegment &segment : longerSegmentList) {
    if (startingOffset + segment.strokeLength > conversionLimit) {
      break;
    }
    startingOffset += segment.strokeLength;
    segmentList.Add(StenoSegment(
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
  BuildSegmentContext context(segmentList, dictionary, orthography);
  buffer.segmentBuilder.CreateSegments(context, startingOffset);
}

void StenoEngine::ConvertText(ConversionBuffer &buffer,
                              StenoSegmentList &segmentList,
                              size_t startingOffset) {
  StenoTokenizer *tokenizer =
      StenoTokenizer::Create(segmentList, startingOffset);
  buffer.keyCodeBuffer.Populate(tokenizer);
  if (placeSpaceAfter && !buffer.keyCodeBuffer.state.joinNext &&
      segmentList.IsNotEmpty()) {
    buffer.keyCodeBuffer.AppendSpace();
  }
  delete tokenizer;
}

//---------------------------------------------------------------------------

void StenoEngine::PrintPaperTapeUndo(size_t undoCount) const {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[StenoStroke::MAX_STRING_LENGTH];
  UNDO_STROKE.ToWideString(buffer);
  Console::Printf(
      "EV {\"event\":\"paper_tape\",\"data\":\"%s\",\"undo\":%zu}\n\n", buffer,
      undoCount);
}

void StenoEngine::PrintPaperTape(
    StenoStroke stroke, const StenoSegmentList &previousSegmentList,
    const StenoSegmentList &nextSegmentList) const {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[256];
  stroke.ToWideString(buffer);
  Console::Printf("EV {\"event\":\"paper_tape\",\"data\":\"%s\"", buffer);

  size_t commonIndex = 0;
  while (commonIndex < previousSegmentList.GetCount() &&
         commonIndex < nextSegmentList.GetCount() &&
         Str::Eq(previousSegmentList[commonIndex].lookup.GetText(),
                 nextSegmentList[commonIndex].lookup.GetText())) {
    ++commonIndex;
  }

  const size_t undoCount = previousSegmentList.GetCount() - commonIndex;
  if (undoCount > 0) {
    Console::Printf(",\"undo\":%zu", undoCount);
  }

  if (commonIndex + 1 == nextSegmentList.GetCount()) {
    const StenoSegment &segment = nextSegmentList[commonIndex];
    const size_t strokeStartIndex =
        nextConversionBuffer.segmentBuilder.GetStateIndex(segment.state);

    const StenoStroke *strokes =
        nextConversionBuffer.segmentBuilder.GetStrokes(strokeStartIndex);
    const size_t length = segment.strokeLength;
    const StenoDictionary *provider =
        dictionary.GetDictionaryForOutline(strokes, length);
    if (provider != nullptr) {
      Console::Printf(",\"dictionary\":\"%J\"", provider->GetName());
    }
  }

  List<StenoSegment> newSegments;
  newSegments.AddCount(nextSegmentList.Skip(commonIndex));
  Console::Printf(",\"definition\":\"");
  StenoTokenizer *tokenizer = StenoTokenizer::Create(newSegments);
  bool isFirstToken = true;
  while (tokenizer->HasMore()) {
    if (isFirstToken) {
      isFirstToken = false;
    } else {
      Console::Printf(" ");
    }
    Console::WriteAsJson(tokenizer->GetNext().text, buffer);
  }
  delete tokenizer;

  Console::Printf("\"}\n\n");
}

void StenoEngine::PrintSuggestions(const StenoSegmentList &previousSegmentList,
                                   const StenoSegmentList &nextSegmentList) {
  if (!IsSuggestionsEnabled()) {
    return;
  }

  // Finger spelling suggestions.
  if (Str::IsFingerSpellingCommand(nextSegmentList.Back().lookup.GetText())) {
    if (state.isManualStateChange || state.joinNext) {
      return;
    }

    PrintFingerSpellingSuggestions(previousSegmentList, nextSegmentList);
    return;
  }

  // General suggestions. Search back up to 8 word segments.
  char *lastLookup = nullptr;
  for (size_t segmentCount = 1; segmentCount < 8; ++segmentCount) {
    Pump();
    char *newLookup =
        PrintSegmentSuggestion(segmentCount, nextSegmentList, lastLookup);
    free(lastLookup);
    lastLookup = newLookup;
    if (!lastLookup) {
      return;
    }
  }
  free(lastLookup);
}

void StenoEngine::PrintFingerSpellingSuggestions(
    const StenoSegmentList &previousSegmentList,
    const StenoSegmentList &nextSegmentList) {
  // Get the last word out of the buffer and look that up.
  char buffer[256];
  char *p = buffer + sizeof(buffer) - 1;
  *p = '\0';
  const StenoKeyCode *skc =
      &nextConversionBuffer.keyCodeBuffer
           .buffer[nextConversionBuffer.keyCodeBuffer.count - 1];

  if (placeSpaceAfter && skc > nextConversionBuffer.keyCodeBuffer.buffer) {
    skc--;
  }

  size_t keyCodeCount = 0;
  while (skc >= nextConversionBuffer.keyCodeBuffer.buffer &&
         !skc->IsWhitespace() && !skc->IsRawKeyCode()) {
    const uint32_t unicode = skc->GetUnicode();
    const size_t length = Utf8Pointer::BytesForCharacterCode(unicode);
    p -= length;
    Utf8Pointer(p).SetAndAdvance(unicode);
    ++keyCodeCount;
    --skc;
  }

  if (keyCodeCount > 1) {
    PrintSuggestion(p, 1, keyCodeCount);
  }
}

void StenoEngine::PrintSuggestion(const char *p, size_t arrowPrefixCount,
                                  size_t strokeThreshold) const {
  StenoReverseDictionaryLookup lookup(strokeThreshold, p);
  ReverseLookup(lookup);
  if (lookup.results.IsEmpty()) {
    return;
  }

  Console::Printf("EV {"
                  "\"event\":\"suggestion\","
                  "\"combine_count\":%zu,"
                  "\"text\":\"%J\","
                  "\"outlines\":[",
                  arrowPrefixCount, p);
  for (size_t i = 0; i < lookup.results.GetCount(); ++i) {
    const StenoReverseDictionaryResult &entry = lookup.results[i];
    Console::Printf(i == 0 ? "\"%T\"" : ",\"%T\"", entry.strokes, entry.length);
  }
  Console::Printf("]");
  if (lookup.AreAllFromSameDictionary()) {
    const StenoDictionary *dictionary = lookup.results.Front().dictionary;
    const char *name = dictionary->GetName();
    if (name[0] != '#') {
      Console::Printf(",\"dictionary\":\"%J\"", name);
    }
  }
  Console::Printf("}\n\n");
}

static bool ShouldShowSuggestions(const List<StenoSegment> &segmentList) {
  // Count the number of suffix "{*!}" entries.
  // There must be more that number of entries before that.
  size_t joinPreviousCount = 0;
  for (const StenoSegment &segment : segmentList.Reverse()) {
    if (!Str::Eq(segment.lookup.GetText(), "{*!}")) {
      break;
    }
    ++joinPreviousCount;
  }
  return segmentList.GetCount() > 2 * joinPreviousCount;
}

static bool HasManualStateChange(const List<StenoSegment> &segmentList) {
  for (const StenoSegment &segment : segmentList) {
    if (segment.state->isManualStateChange) {
      return true;
    }
  }
  return false;
}

char *StenoEngine::PrintSegmentSuggestion(size_t segmentCount,
                                          const StenoSegmentList &segmentList,
                                          char *lastLookup) {

  size_t startSegmentIndex = segmentList.GetCount();
  StenoState lastState = state;
  for (size_t i = 0; i < segmentCount; ++i) {
    if (startSegmentIndex == 0) {
      return nullptr;
    }
    while (startSegmentIndex != 0) {
      --startSegmentIndex;
      if (segmentList[startSegmentIndex].ContainsKeyCode()) {
        return nullptr;
      }

      // Consider it a segment start if it isn't a suffix stroke and it isn't
      // a fingerspelling joined to a previous fingerspelling.
      // This will still give suggestions after prefixes, e.g.
      //   overwatching: AUFR/WAFP/-G will suggest to combine WAFPG
      const char *startSegmentText =
          segmentList[startSegmentIndex].lookup.GetText();
      if (!Str::IsJoinPrevious(startSegmentText) &&
          (startSegmentIndex == 0 ||
           !Str::IsFingerSpellingCommand(startSegmentText) ||
           !Str::IsFingerSpellingCommand(
               segmentList[startSegmentIndex - 1].lookup.GetText()))) {
        break;
      }
    }
    lastState = *segmentList[startSegmentIndex].state;
  }

  if (segmentList.GetCount() - startSegmentIndex >=
      PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT) {
    return nullptr;
  }

  List<StenoSegment> testSegments;
  testSegments.AddCount(segmentList.Skip(startSegmentIndex));

  size_t strokeThresholdCount = 0;
  for (const StenoSegment &segment : segmentList.Skip(startSegmentIndex)) {
    strokeThresholdCount += segment.strokeLength;
  }

  StenoTokenizer *tokenizer = StenoTokenizer::Create(testSegments);
  previousConversionBuffer.keyCodeBuffer.Populate(tokenizer);
  delete tokenizer;

  if (!ShouldShowSuggestions(testSegments)) {
    return Str::Dup("");
  }

  char *lookup =
      HasManualStateChange(testSegments) ||
              Str::HasPrefix(testSegments.Back().lookup.GetText(), "{:")
          ? previousConversionBuffer.keyCodeBuffer.ToString()
          : previousConversionBuffer.keyCodeBuffer.ToUnresolvedString();

  char *spaceRemoved = *lookup == ' ' ? lookup + 1 : lookup;

  // Special case {*!} and function calls at the start to avoid suggestions.
  if (Str::Eq(testSegments[0].lookup.GetText(), "{*!}") ||
      Str::HasPrefix(testSegments[0].lookup.GetText(), "{:")) {
    return lookup;
  }

  bool printSuggestion = *spaceRemoved != '\0' &&
                         (startSegmentIndex != segmentList.GetCount() - 1 ||
                          strokeThresholdCount != 1);

  if (state.joinNext) {
    bool usePrefixSyntax = true;
    if (segmentList.GetCount() >= 2) {
      // If the previous state is the same, and the new state ends in a space,
      // truncate the space and treat it as a non-space lookup.
      if (*segmentList.Back().state == state) {
        const size_t length = Str::Length(spaceRemoved);
        if (length != 0 && spaceRemoved[length - 1] == ' ') {
          spaceRemoved = Str::DupN(spaceRemoved, length - 1);
          free(lookup);
          lookup = spaceRemoved;
          usePrefixSyntax = false;
        }
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

  if (printSuggestion) {
    PrintSuggestion(spaceRemoved, strokeThresholdCount, strokeThresholdCount);
  }

  return lookup;
}

void StenoEngine::PrintTextLog(
    const StenoKeyCodeBuffer &previousKeyCodeBuffer,
    const StenoKeyCodeBuffer &nextKeyCodeBuffer) const {
  if (!IsTextLogEnabled()) {
    return;
  }

  const StenoKeyCode *previousData = previousKeyCodeBuffer.buffer;
  const size_t previousLength = previousKeyCodeBuffer.count;
  const StenoKeyCode *nextData = nextKeyCodeBuffer.buffer;
  const size_t nextLength = nextKeyCodeBuffer.count;
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

  Console::Printf("EV {\"event\":\"text_log\",\"text\":\"");
  static const char BACKSPACES[] =
      "\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b";

  while (backspaceCount > 0) {
    const size_t writeCount = backspaceCount > 16 ? 16 : backspaceCount;
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

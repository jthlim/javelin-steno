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
                                       StenoKeyCodeBuffer *keyCodeBuffer,
                                       StenoSegmentList *segments,
                                       size_t startingOffset)
      : engine(engine), keyCodeBuffer(keyCodeBuffer), segments(segments),
        startingOffset(startingOffset) {}

  StenoEngine *engine;
  StenoKeyCodeBuffer *keyCodeBuffer;
  StenoSegmentList *segments;
  size_t startingOffset;

  void ConvertText() {
    engine->ConvertText(*keyCodeBuffer, *segments, startingOffset);
  }

  static void ConvertTextEntryPoint(void *data) {
    ((UpdateNormalModeTextBufferThreadData *)data)->ConvertText();
  }
};

#endif

size_t StenoEngine::GetStartingStrokeForNormalModeProcessing() const {
  const size_t maximumConversionStrokes = dictionary.GetMaximumOutlineLength();
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
    const size_t MAX_EXTRA_STROKES = 4;
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

void StenoEngine::ProcessNormalModeStroke(StenoStroke stroke) {
#if ENABLE_DICTIONARY_STATS
  StenoDictionary::ResetStats();
#endif

  history.PruneIfFull();

#if ENABLE_PROFILE
  uint32_t t0 = Clock::GetMicroseconds();
#endif

  const size_t previousSourceStrokeCount = history.GetCount();
  const size_t startingStroke = GetStartingStrokeForNormalModeProcessing();

  // ConversionCount adds one as a stroke is added below, but that call
  // also needs to be provided with the conversion count.
  const size_t conversionCount = history.GetCount() + 1 - startingStroke;

  history.Add(stroke, state, conversionCount);

  StenoSegmentList nextSegments;
  CreateSegments(history.GetCount(), nextConversionBuffer, conversionCount,
                 nextSegments, true);
  history.UpdateDefinitionBoundaries(history.GetCount() - conversionCount,
                                     nextSegments);

#if ENABLE_PROFILE
  uint32_t t1 = Clock::GetMicroseconds();
#endif

  StenoSegmentList previousSegments;
  if (nextConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(previousSourceStrokeCount, previousConversionBuffer,
                   conversionCount - 1, previousSegments, false);
  } else {
    CreateSegmentsUsingLongerResult(previousSourceStrokeCount,
                                    previousConversionBuffer,
                                    conversionCount - 1, previousSegments,
                                    nextConversionBuffer, nextSegments);
  }

#if ENABLE_PROFILE
  uint32_t t2 = Clock::GetMicroseconds();
#endif

  size_t startingOffset = StenoSegmentList::GetCommonStartingSegmentsCount(
      previousSegments, nextSegments);
  if (startingOffset > 0 && placeSpaceAfter) {
    --startingOffset;
  }

#if ENABLE_PROFILE
  uint32_t t3 = Clock::GetMicroseconds();
#endif

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData previousThreadData(
      this, &previousConversionBuffer.keyCodeBuffer, &previousSegments,
      startingOffset);
  UpdateNormalModeTextBufferThreadData nextThreadData(
      this, &nextConversionBuffer.keyCodeBuffer, &nextSegments, startingOffset);

  RunParallel(&UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &previousThreadData,
              &UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &nextThreadData);
#else
  ConvertText(previousConversionBuffer.keyCodeBuffer, previousSegments,
              startingOffset);
  ConvertText(nextConversionBuffer.keyCodeBuffer, nextSegments, startingOffset);
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
  PrintPaperTape(stroke, previousSegments, nextSegments);

  if (nextConversionBuffer.keyCodeBuffer.addTranslationCount >
      previousConversionBuffer.keyCodeBuffer.addTranslationCount) {
    history.SetBackNoCombineUndo();
    InitiateAddTranslationMode(
        nextConversionBuffer.keyCodeBuffer.addTranslationText);
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
    PrintSuggestions(previousSegments, nextSegments);
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
      history.IsEmpty() ? 0 : history.Back().conversionCount;

  const size_t undoCount = history.GetUndoCount(maximumConversionStrokes);
  if (undoCount == 0) {
    Key::Tap(KeyCode::BACKSPACE);
    PrintPaperTapeUndo(0);
    return;
  }

  const size_t startingStroke =
      history.GetStartingStroke(maximumConversionStrokes);
  const size_t conversionCount = history.GetCount() - startingStroke;

  StenoSegmentList previousSegments;
  CreateSegments(history.GetCount(), previousConversionBuffer, conversionCount,
                 previousSegments, false);

  state = history.Back(undoCount).state;
  state.shouldCombineUndo = false;
  state.isManualStateChange = false;
  history.RemoveBack(undoCount);

  const size_t nextConversionCount =
      undoCount >= conversionCount ? 0 : conversionCount - undoCount;

  StenoSegmentList nextSegments;
  if (previousConversionBuffer.segmentBuilder.HasModifiedStrokeHistory()) {
    CreateSegments(history.GetCount(), nextConversionBuffer,
                   nextConversionCount, nextSegments, true);

  } else {
    CreateSegmentsUsingLongerResult(history.GetCount(), nextConversionBuffer,
                                    nextConversionCount, nextSegments,
                                    previousConversionBuffer, previousSegments);
  }

  history.UpdateDefinitionBoundaries(history.GetCount() - nextConversionCount,
                                     nextSegments);

  size_t startingOffset = StenoSegmentList::GetCommonStartingSegmentsCount(
      previousSegments, nextSegments);
  if (startingOffset > 0 && placeSpaceAfter) {
    --startingOffset;
  }

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData previousThreadData(
      this, &previousConversionBuffer.keyCodeBuffer, &previousSegments,
      startingOffset);
  UpdateNormalModeTextBufferThreadData nextThreadData(
      this, &nextConversionBuffer.keyCodeBuffer, &nextSegments, startingOffset);

  RunParallel(&UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &previousThreadData,
              &UpdateNormalModeTextBufferThreadData::ConvertTextEntryPoint,
              &nextThreadData);
#else
  ConvertText(previousConversionBuffer.keyCodeBuffer, previousSegments,
              startingOffset);
  ConvertText(nextConversionBuffer.keyCodeBuffer, nextSegments, startingOffset);
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
                                 StenoSegmentList &segments,
                                 bool allowSetValue) {
  buffer.segmentBuilder.TransferFrom(history, sourceStrokeCount,
                                     conversionLimit);
  BuildSegmentContext context(segments, *this, allowSetValue);
  buffer.segmentBuilder.CreateSegments(context);
}

void StenoEngine::CreateSegmentsUsingLongerResult(
    size_t sourceStrokeCount, ConversionBuffer &buffer, size_t conversionLimit,
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
  BuildSegmentContext context(segments, *this, false);
  buffer.segmentBuilder.CreateSegments(context, startingOffset);
}

void StenoEngine::ConvertText(StenoKeyCodeBuffer &keyCodeBuffer,
                              StenoSegmentList &segments,
                              size_t startingOffset) {
  StenoTokenizer *tokenizer = StenoTokenizer::Create(segments, startingOffset);
  keyCodeBuffer.Populate(tokenizer);
  if (placeSpaceAfter && !keyCodeBuffer.state.joinNext &&
      segments.IsNotEmpty()) {
    keyCodeBuffer.AppendSpace();
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

void StenoEngine::PrintPaperTape(StenoStroke stroke,
                                 const StenoSegmentList &previousSegments,
                                 const StenoSegmentList &nextSegments) const {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[256];
  stroke.ToWideString(buffer);
  Console::Printf("EV {\"event\":\"paper_tape\",\"data\":\"%s\"", buffer);

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
    Console::Printf(",\"undo\":%zu", undoCount);
  }

  const StenoStroke *strokes =
      nextConversionBuffer.segmentBuilder.GetStrokes(startingStrokeIndex);
  const size_t length = segment.strokeLength;
  const StenoDictionary *provider =
      dictionary.GetDictionaryForOutline(strokes, length);
  if (provider != nullptr) {
    Console::Printf(",\"dictionary\":\"%J\"", provider->GetName());
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
    Console::Printf(",\"definition\":\"%J\"}\n\n", writer.GetBuffer());
  } else {
    Console::Printf(",\"definition\":\"%J\"}\n\n", lookup);
  }
}

void StenoEngine::PrintSuggestions(const StenoSegmentList &previousSegments,
                                   const StenoSegmentList &nextSegments) {
  if (!IsSuggestionsEnabled()) {
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
  char *lastLookup = nullptr;
  for (size_t segmentCount = 1; segmentCount < 8; ++segmentCount) {
    Pump();
    char *newLookup =
        PrintSegmentSuggestion(segmentCount, nextSegments, lastLookup);
    free(lastLookup);
    lastLookup = newLookup;
    if (!lastLookup) {
      return;
    }
  }
  free(lastLookup);
}

void StenoEngine::PrintFingerSpellingSuggestions(
    const StenoSegmentList &previousSegments,
    const StenoSegmentList &nextSegments) {
  // Get the last word out of the buffer and look that up.
  char buffer[256];
  char *p = buffer + sizeof(buffer) - 2;
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

static bool ShouldShowSuggestions(const List<StenoSegment> &segments) {
  // Count the number of suffix "{*!}" entries.
  // There must be more that number of entries before that.
  size_t joinPreviousCount = 0;
  for (const StenoSegment &segment : segments.Reverse()) {
    if (!Str::Eq(segment.lookup.GetText(), "{*!}")) {
      break;
    }
    ++joinPreviousCount;
  }
  return segments.GetCount() > 2 * joinPreviousCount;
}

static bool HasManualStateChange(const List<StenoSegment> &segments) {
  for (const StenoSegment &segment : segments) {
    if (segment.state->isManualStateChange) {
      return true;
    }
  }
  return false;
}

char *StenoEngine::PrintSegmentSuggestion(size_t segmentCount,
                                          const StenoSegmentList &segments,
                                          char *lastLookup) {

  size_t startSegmentIndex = segments.GetCount();
  StenoState lastState = state;
  for (size_t i = 0; i < segmentCount; ++i) {
    if (startSegmentIndex == 0) {
      return nullptr;
    }
    while (startSegmentIndex != 0) {
      --startSegmentIndex;
      if (segments[startSegmentIndex].ContainsKeyCode()) {
        return nullptr;
      }

      if (segments[startSegmentIndex].lookup ==
          StenoDictionaryLookupResult::NO_OP) {
        return nullptr;
      }

      // Consider it a segment start if it isn't a suffix stroke and it isn't
      // a fingerspelling joined to a previous fingerspelling.
      // This will still give suggestions after prefixes, e.g.
      //   overwatching: AUFR/WAFP/-G will suggest to combine WAFPG
      const char *startSegmentText =
          segments[startSegmentIndex].lookup.GetText();

      if (!Str::IsJoinPrevious(startSegmentText) &&
          (startSegmentIndex == 0 ||
           !Str::IsFingerSpellingCommand(startSegmentText) ||
           !Str::IsFingerSpellingCommand(
               segments[startSegmentIndex - 1].lookup.GetText()))) {
        break;
      }
    }
    lastState = *segments[startSegmentIndex].state;
  }

  if (segments.GetCount() - startSegmentIndex >=
      PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT) {
    return nullptr;
  }

  List<StenoSegment> testSegments;
  testSegments.AddCount(segments.Skip(startSegmentIndex));

  size_t strokeThresholdCount = 0;
  for (const StenoSegment &segment : segments.Skip(startSegmentIndex)) {
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

  bool printSuggestion =
      *spaceRemoved != '\0' && (startSegmentIndex != segments.GetCount() - 1 ||
                                strokeThresholdCount != 1);

  if (state.joinNext) {
    bool usePrefixSyntax = true;
    if (segments.GetCount() >= 2) {
      // If the previous state is the same, and the new state ends in a space,
      // truncate the space and treat it as a non-space lookup.
      if (*segments.Back().state == state) {
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

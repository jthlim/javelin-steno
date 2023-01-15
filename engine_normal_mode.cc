//---------------------------------------------------------------------------

#include "console.h"
#include "engine.h"
#include "key_code.h"
#include "segment.h"
#include "str.h"
#include "thread.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

#if JAVELIN_THREADS

struct StenoEngine::UpdateNormalModeTextBufferThreadData {
  StenoEngine *engine;
  size_t sourceStrokeCount;
  ConversionBuffer *conversionBuffer;
  StenoSegmentList segmentList;
  size_t conversionLimit;

  void Run() {
    engine->UpdateNormalModeTextBuffer(sourceStrokeCount, *conversionBuffer,
                                       conversionLimit, segmentList);
  }
  static void EntryPoint(void *data) {
    ((UpdateNormalModeTextBufferThreadData *)data)->Run();
  }
};

#endif

void StenoEngine::ProcessNormalModeStroke(StenoStroke stroke) {
  history.ShiftIfFull();

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData threadData[2];
  threadData[0].engine = this;
  threadData[0].sourceStrokeCount = history.GetCount();
  threadData[0].conversionBuffer = &previousConversionBuffer;
  threadData[0].conversionLimit = SEGMENT_CONVERSION_LIMIT - 1;

  history.Add(stroke, state);

  threadData[1].engine = this;
  threadData[1].sourceStrokeCount = history.GetCount();
  threadData[1].conversionBuffer = &nextConversionBuffer;
  threadData[1].conversionLimit = SEGMENT_CONVERSION_LIMIT;

  RunParallel(&UpdateNormalModeTextBufferThreadData::EntryPoint, &threadData[0],
              &UpdateNormalModeTextBufferThreadData::EntryPoint,
              &threadData[1]);

  StenoSegmentList &previousSegmentList = threadData[0].segmentList;
  StenoSegmentList &nextSegmentList = threadData[1].segmentList;
#else
  StenoSegmentList previousSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), previousConversionBuffer,
                             SEGMENT_CONVERSION_LIMIT - 1, previousSegmentList);

  history.Add(stroke, state);

  StenoSegmentList nextSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), nextConversionBuffer,
                             SEGMENT_CONVERSION_LIMIT, nextSegmentList);
#endif

  state = nextConversionBuffer.keyCodeBuffer.state;
  state.shouldCombineUndo = false;
  state.isManualStateChange = false;

  if (nextConversionBuffer.keyCodeBuffer.addTranslationCount >
      previousConversionBuffer.keyCodeBuffer.addTranslationCount) {
    PrintPaperTape(stroke, previousSegmentList, nextSegmentList);

    history.Pop();
    InitiateAddTranslationMode();
    return;
  }

  bool printSuggestions = true;
  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    history.SetBackCombineUndo();

    if (previousConversionBuffer.keyCodeBuffer.count ==
        nextConversionBuffer.keyCodeBuffer.count) {
      history.SetBackHasManualStateChange();
      printSuggestions = false;
    }
  }

  PrintPaperTape(stroke, previousSegmentList, nextSegmentList);
  if (printSuggestions) {
    PrintSuggestions(previousSegmentList, nextSegmentList);
  }

  if (nextConversionBuffer.keyCodeBuffer.resetStateCount >
      previousConversionBuffer.keyCodeBuffer.resetStateCount) {
    ResetState();
    return;
  }
}

void StenoEngine::ProcessNormalModeUndo() {
  size_t undoCount = history.GetUndoCount(SEGMENT_CONVERSION_LIMIT);
  if (undoCount == 0) {
    Key::Press(KeyCode::BACKSPACE);
    Key::Release(KeyCode::BACKSPACE);
    PrintPaperTapeUndo(0);
    return;
  }

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData threadData[2];
  threadData[0].engine = this;
  threadData[0].sourceStrokeCount = history.GetCount();
  threadData[0].conversionBuffer = &previousConversionBuffer;
  threadData[0].conversionLimit = SEGMENT_CONVERSION_LIMIT;

  threadData[1].engine = this;
  threadData[1].sourceStrokeCount = history.GetCount() - undoCount;
  threadData[1].conversionBuffer = &nextConversionBuffer;
  threadData[1].conversionLimit = SEGMENT_CONVERSION_LIMIT - undoCount;

  RunParallel(&UpdateNormalModeTextBufferThreadData::EntryPoint, &threadData[0],
              &UpdateNormalModeTextBufferThreadData::EntryPoint,
              &threadData[1]);

  state = history.BackState(undoCount);
  state.shouldCombineUndo = false;
  history.PopCount(undoCount);
#else
  StenoSegmentList previousSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), previousConversionBuffer,
                             SEGMENT_CONVERSION_LIMIT, previousSegmentList);

  state = history.BackState(undoCount);
  state.shouldCombineUndo = false;
  history.PopCount(undoCount);

  StenoSegmentList nextSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), nextConversionBuffer,
                             SEGMENT_CONVERSION_LIMIT - undoCount,
                             nextSegmentList);
#endif

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  PrintPaperTapeUndo(undoCount);
}

void StenoEngine::UpdateNormalModeTextBuffer(size_t sourceStrokeCount,
                                             ConversionBuffer &buffer,
                                             size_t conversionLimit,
                                             StenoSegmentList &segmentList) {
  buffer.strokeHistory.TransferFrom(history, sourceStrokeCount,
                                    conversionLimit);
  BuildSegmentContext context(segmentList, dictionary, orthography);

  buffer.strokeHistory.CreateSegments(context);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.keyCodeBuffer.Populate(tokenizer);
  delete tokenizer;
}

//---------------------------------------------------------------------------

void StenoEngine::PrintPaperTapeUndo(size_t undoCount) {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[32];
  UNDO_STROKE.ToWideString(buffer);
  Console::Printf(
      "EV {\"event\":\"paper_tape\",\"data\":\"%s\",\"undo\":%zu}\n\n", buffer,
      undoCount);
}

void StenoEngine::PrintPaperTape(StenoStroke stroke,
                                 const StenoSegmentList &previousSegmentList,
                                 const StenoSegmentList &nextSegmentList) {
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

  size_t undoCount = previousSegmentList.GetCount() - commonIndex;
  if (undoCount > 0) {
    Console::Printf(",\"undo\":%zu", undoCount);
  }

  StenoSegmentList newSegments;
  for (size_t i = commonIndex; i < nextSegmentList.GetCount(); ++i) {
    newSegments.Add(nextSegmentList[i]);
  }

  Console::Printf(",\"text\":\"");
  StenoTokenizer *tokenizer = newSegments.CreateTokenizer();
  bool isFirstToken = true;
  while (tokenizer->HasMore()) {
    if (isFirstToken) {
      isFirstToken = false;
    } else {
      Console::Write(" ", 1);
    }
    Console::WriteAsJson(tokenizer->GetNext().text, buffer);
  }
  delete tokenizer;

  newSegments.Reset();
  Console::Write("\"}\n\n", 4);
}

void StenoEngine::PrintSuggestions(const StenoSegmentList &previousSegmentList,
                                   const StenoSegmentList &nextSegmentList) {
  if (!IsPaperTapeEnabled() && !IsSuggestionsEnabled()) {
    return;
  }

  if (state.isManualStateChange || state.joinNext) {
    return;
  }

  char buffer[256];

  // Finger spelling suggestions.
  if (Str::IsFingerSpellingCommand(nextSegmentList.Back().lookup.GetText())) {
    // Get the last word out of the buffer and look that up.
    char *p = buffer + sizeof(buffer) - 1;
    *p = '\0';
    const StenoKeyCode *skc =
        &nextConversionBuffer.keyCodeBuffer
             .buffer[nextConversionBuffer.keyCodeBuffer.count - 1];
    size_t keyCodeCount = 0;
    while (skc >= nextConversionBuffer.keyCodeBuffer.buffer &&
           !skc->IsWhitespace() && !skc->IsRawKeyCode()) {
      uint32_t unicode = skc->GetUnicode();
      size_t length = Utf8Pointer::BytesForCharacterCode(unicode);
      p -= length;
      Utf8Pointer(p).Set(unicode);
      ++keyCodeCount;
      --skc;
    }

    if (keyCodeCount > 1) {
      PrintSuggestion(p, 1, buffer, keyCodeCount);
    }
    return;
  }

  // General suggestions. Search back up to 8 word segments.
  char *lastLookup = nullptr;
  for (size_t wordCount = 1; wordCount < 8; ++wordCount) {
    char *newLookup =
        PrintSegmentSuggestion(wordCount, nextSegmentList, buffer, lastLookup);
    free(lastLookup);
    lastLookup = newLookup;
    if (!lastLookup) {
      return;
    }
  }
  free(lastLookup);
}

void StenoEngine::PrintSuggestion(const char *p, size_t arrowPrefixCount,
                                  char *buffer, size_t strokeThreshold) {
  StenoReverseDictionaryLookup result(strokeThreshold, p);
  ReverseLookup(result);
  if (result.resultCount == 0) {
    return;
  }

  Console::Printf(
      "EV {\"event\":\"suggestion\",\"combine_count\":%zu,\"text\":\"",
      arrowPrefixCount);
  Console::WriteAsJson(p, buffer);
  Console::Printf("\",\"outlines\":[");
  for (size_t i = 0; i < result.resultCount; ++i) {
    const StenoReverseDictionaryResult &lookup = result.results[i];
    StenoStroke::ToString(lookup.strokes, lookup.length, buffer);
    Console::Printf(i == 0 ? "\"%s\"" : ",\"%s\"", buffer);
  }
  Console::Printf("]}\n\n");
}

char *StenoEngine::PrintSegmentSuggestion(size_t wordCount,
                                          const StenoSegmentList &segmentList,
                                          char *buffer, char *lastLookup) {

  size_t startSegmentIndex = segmentList.GetCount();
  StenoState lastState = state;
  for (size_t i = 0; i < wordCount; ++i) {
    if (startSegmentIndex == 0) {
      return nullptr;
    }
    while (startSegmentIndex != 0) {
      --startSegmentIndex;
      if (segmentList[startSegmentIndex].ContainsKeyCode()) {
        return nullptr;
      }

      // Consider it a word start if it isn't a suffix stroke.
      // This will still give suggestions after prefixes, e.g.
      //   overwatching: AUFR/WAFP/-G will suggest to combine WAFPG
      if (!Str::IsJoinPrevious(
              segmentList[startSegmentIndex].lookup.GetText())) {
        break;
      }
    }
    lastState = *segmentList[startSegmentIndex].state;
  }

  if (segmentList.GetCount() - startSegmentIndex >=
      PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT) {
    return nullptr;
  }

  StenoSegmentList testSegments;

  size_t strokeThresholdCount = 0;
  for (size_t i = startSegmentIndex; i < segmentList.GetCount(); ++i) {
    testSegments.Add(segmentList[i]);
    strokeThresholdCount += segmentList[i].strokeLength;
  }

  StenoTokenizer *tokenizer = testSegments.CreateTokenizer();
  previousConversionBuffer.keyCodeBuffer.Populate(tokenizer);
  delete tokenizer;

  // Special case {*!} to avoid suggestions.
  if (testSegments.GetCount() == 2 &&
      (Str::Eq(testSegments[0].lookup.GetText(), "{*!}") ||
       Str::Eq(testSegments[1].lookup.GetText(), "{*!}"))) {
    testSegments.Reset();
    return Str::Dup("");
  }

  char *lookup;

  bool continueLookups = true;
  if (testSegments[0].state->isManualStateChange) {
    lookup = previousConversionBuffer.keyCodeBuffer.ToString();
    continueLookups = false;
  } else {
    lookup = previousConversionBuffer.keyCodeBuffer.ToUnresolvedString();
  }
  char *spaceRemoved = *lookup == ' ' ? lookup + 1 : lookup;

  testSegments.Reset();

  bool printSuggestion = true;
  if (lastLookup) {
    char *lastLookupSpaceRemoved =
        *lastLookup == ' ' ? lastLookup + 1 : lastLookup;

    printSuggestion = !Str::Eq(spaceRemoved, lastLookupSpaceRemoved);
  }

  if (printSuggestion && (startSegmentIndex != segmentList.GetCount() - 1 ||
                          strokeThresholdCount != 1)) {
    // No need to check if there's a single stroke producing the output.

    PrintSuggestion(spaceRemoved, strokeThresholdCount, buffer,
                    strokeThresholdCount);
  }

  if (!continueLookups) {
    free(lookup);
    return nullptr;
  }
  return lookup;
}

//---------------------------------------------------------------------------

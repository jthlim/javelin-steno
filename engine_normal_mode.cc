//---------------------------------------------------------------------------

#include "console.h"
#include "engine.h"
#include "segment.h"
#include "str.h"
#include "thread.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

#if JAVELIN_THREADS

struct StenoEngine::UpdateNormalModeTextBufferThreadData {
  StenoEngine *engine;
  size_t maximumChordCount;
  StenoKeyCodeBuffer *buffer;
  StenoSegmentList segmentList;
  size_t chordLength;

  static void EntryPoint(void *data);
};

void StenoEngine::UpdateNormalModeTextBufferThreadData::EntryPoint(void *data) {
  UpdateNormalModeTextBufferThreadData *threadData =
      (UpdateNormalModeTextBufferThreadData *)data;

  threadData->engine->UpdateNormalModeTextBuffer(
      threadData->maximumChordCount, *threadData->buffer,
      threadData->chordLength, threadData->segmentList);
}

#endif

void StenoEngine::ProcessNormalModeChord(StenoChord chord) {
  history.ShiftIfFull();

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData threadData[2];
  threadData[0].engine = this;
  threadData[0].maximumChordCount = history.GetCount();
  threadData[0].buffer = &previousKeyCodeBuffer;
  threadData[0].chordLength = SEGMENT_CONVERSION_LIMIT - 1;

  history.Add(chord, state);

  threadData[1].engine = this;
  threadData[1].maximumChordCount = history.GetCount();
  threadData[1].buffer = &nextKeyCodeBuffer;
  threadData[1].chordLength = SEGMENT_CONVERSION_LIMIT;

  RunParallel(&UpdateNormalModeTextBufferThreadData::EntryPoint, &threadData[0],
              &UpdateNormalModeTextBufferThreadData::EntryPoint,
              &threadData[1]);

  StenoSegmentList &previousSegmentList = threadData[0].segmentList;
  StenoSegmentList &nextSegmentList = threadData[1].segmentList;
#else
  StenoSegmentList previousSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), previousKeyCodeBuffer,
                             SEGMENT_CONVERSION_LIMIT - 1, previousSegmentList);

  history.Add(chord, state);

  StenoSegmentList nextSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), nextKeyCodeBuffer,
                             SEGMENT_CONVERSION_LIMIT, nextSegmentList);
#endif

  state = nextKeyCodeBuffer.state;
  state.hasManualStateChange = false;

  if (nextKeyCodeBuffer.addTranslationCount >
      previousKeyCodeBuffer.addTranslationCount) {
    PrintPaperTape(chord, previousSegmentList, nextSegmentList, false);

    history.Pop();
    InitiateAddTranslationMode();
    return;
  }

  if (nextKeyCodeBuffer.resetStateCount >
      previousKeyCodeBuffer.resetStateCount) {
    ResetState();
    return;
  }

  if (!emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer) &&
      nextSegmentList.GetCount() > previousSegmentList.GetCount()) {
    history.Pop();

    if (previousKeyCodeBuffer.count == nextKeyCodeBuffer.count) {
      state.hasManualStateChange = true;
    }
  }

  PrintPaperTape(chord, previousSegmentList, nextSegmentList, false);
}

void StenoEngine::ProcessNormalModeUndo() {
  if (history.IsEmpty()) {
    return;
  }

#if JAVELIN_THREADS
  UpdateNormalModeTextBufferThreadData threadData[2];
  threadData[0].engine = this;
  threadData[0].maximumChordCount = history.GetCount();
  threadData[0].buffer = &previousKeyCodeBuffer;
  threadData[0].chordLength = SEGMENT_CONVERSION_LIMIT;

  threadData[1].engine = this;
  threadData[1].maximumChordCount = history.GetCount() - 1;
  threadData[1].buffer = &nextKeyCodeBuffer;
  threadData[1].chordLength = SEGMENT_CONVERSION_LIMIT - 1;

  RunParallel(&UpdateNormalModeTextBufferThreadData::EntryPoint, &threadData[0],
              &UpdateNormalModeTextBufferThreadData::EntryPoint,
              &threadData[1]);

  state = history.BackState();
  history.Pop();

  StenoSegmentList &previousSegmentList = threadData[0].segmentList;
  StenoSegmentList &nextSegmentList = threadData[1].segmentList;
#else
  StenoSegmentList previousSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(), previousKeyCodeBuffer,
                             SEGMENT_CONVERSION_LIMIT, previousSegmentList);
  state = history.BackState();
  history.Pop();

  StenoSegmentList nextSegmentList;
  UpdateNormalModeTextBuffer(history.GetCount(),

                             nextKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT - 1,
                             nextSegmentList);
#endif

  emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer);

  PrintPaperTape(UNDO_CHORD, previousSegmentList, nextSegmentList, true);
}

void StenoEngine::UpdateNormalModeTextBuffer(size_t maximumChordCount,
                                             StenoKeyCodeBuffer &buffer,
                                             size_t chordLength,
                                             StenoSegmentList &segmentList) {
  BuildSegmentContext context(segmentList, dictionary,
                              dictionary.GetMaximumMatchLength(), orthography,
                              maximumChordCount);

  history.CreateSegments(context, chordLength);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.Populate(tokenizer);
  delete tokenizer;
}

//---------------------------------------------------------------------------

void StenoEngine::PrintPaperTape(StenoChord chord,
                                 const StenoSegmentList &previousSegmentList,
                                 const StenoSegmentList &nextSegmentList,
                                 bool isUndo) {
  if (!IsPaperTapeEnabled()) {
    return;
  }

  char buffer[256];
  chord.ToWideString(buffer);
  Console::Printf("PT | %s | ", buffer);

  size_t commonIndex = 0;
  while (commonIndex < previousSegmentList.GetCount() &&
         commonIndex < nextSegmentList.GetCount() &&
         Str::Eq(previousSegmentList[commonIndex].lookup.GetText(),
                 nextSegmentList[commonIndex].lookup.GetText())) {
    ++commonIndex;
  }

  for (size_t i = commonIndex; i < previousSegmentList.GetCount(); ++i) {
    Console::Write("*", 1);
  }

  StenoSegmentList newSegments;
  for (size_t i = commonIndex; i < nextSegmentList.GetCount(); ++i) {
    newSegments.Add(nextSegmentList[i]);
  }

  StenoTokenizer *tokenizer = newSegments.CreateTokenizer();
  bool isFirstToken = true;
  while (tokenizer->HasMore()) {
    if (isFirstToken) {
      isFirstToken = false;
    } else {
      Console::Write(" ", 1);
    }
    char *end = Str::WriteJson(buffer, tokenizer->GetNext().text);
    Console::Write(buffer, end - buffer);
  }
  delete tokenizer;

  newSegments.Reset();
  Console::Write("\n\n", 3);

  if (isUndo) {
    return;
  }

  // Finger spelling suggestions.
  if (Str::IsFingerSpellingCommand(nextSegmentList.Back().lookup.GetText())) {
    // Get the last word out of the buffer and look that up.
    char *p = buffer + sizeof(buffer) - 1;
    *p = '\0';
    const StenoKeyCode *skc =
        &nextKeyCodeBuffer.buffer[nextKeyCodeBuffer.count - 1];
    size_t keyCodeCount = 0;
    while (skc >= nextKeyCodeBuffer.buffer && !skc->IsWhitespace() &&
           !skc->IsRawKeyCode()) {
      uint32_t unicode = skc->GetUnicode();
      size_t length = Utf8Pointer::BytesForCharacterCode(unicode);
      p -= length;
      Utf8Pointer(p).Set(unicode);
      ++keyCodeCount;
      --skc;
    }

    if (keyCodeCount > 1) {
      PrintPaperTapeSuggestion(p, 1, buffer, keyCodeCount);
    }
    return;
  }

  // General suggestions. Search back up to 8 word segments.
  char *lastLookup = nullptr;
  for (size_t wordCount = 1; wordCount < 8; ++wordCount) {
    lastLookup = PrintPaperTapeSegmentSuggestion(wordCount, nextSegmentList,
                                                 buffer, lastLookup);
    if (lastLookup == nullptr) {
      return;
    }
  }
}

void StenoEngine::PrintPaperTapeSuggestion(const char *p,
                                           size_t arrowPrefixCount,
                                           char *buffer,
                                           size_t strokeThreshold) {
  static const char ARROWS[] = ">>>>>>>>>>>>";

  StenoReverseDictionaryLookup result(strokeThreshold, p);
  dictionary.ReverseLookup(result);
  if (result.resultCount == 0) {
    return;
  }

  StenoChord().ToWideString(buffer);
  Console::Printf("PT   %s   %s", buffer,
                  ARROWS + sizeof(ARROWS) - 1 - arrowPrefixCount);

  const StenoChord *chords = result.chords;
  for (size_t i = 0; i < result.resultCount; ++i) {
    size_t length = result.resultLengths[i];

    StenoChord::ToString(chords, length, buffer);
    Console::Printf(" %s", buffer);

    chords += length;
  }

  Console::Write("\n\n", 3);
}

char *StenoEngine::PrintPaperTapeSegmentSuggestion(
    size_t wordCount, const StenoSegmentList &segmentList, char *buffer,
    char *lastLookup) {

  size_t startSegmentIndex = segmentList.GetCount();
  for (size_t i = 0; i < wordCount; ++i) {
    if (startSegmentIndex == 0) {
      free(lastLookup);
      return nullptr;
    }
    while (startSegmentIndex != 0) {
      --startSegmentIndex;
      if (segmentList[startSegmentIndex].ContainsKeyCode()) {
        free(lastLookup);
        return nullptr;
      }

      // It's a word start if the current doesn't join with the previous,
      // and the previous doesn't join with the next.
      if (!Str::IsJoinPrevious(
              segmentList[startSegmentIndex].lookup.GetText()) &&
          (startSegmentIndex == 0 ||
           !Str::IsJoinNext(
               segmentList[startSegmentIndex - 1].lookup.GetText()))) {
        break;
      }
    }
  }

  if (segmentList.GetCount() - startSegmentIndex >=
      PAPER_TAPE_SUGGESTION_SEGMENT_LIMIT) {
    free(lastLookup);
    return nullptr;
  }

  StenoSegmentList testSegments;

  size_t chordThresholdCount = 0;
  for (size_t i = startSegmentIndex; i < segmentList.GetCount(); ++i) {
    testSegments.Add(segmentList[i]);
    chordThresholdCount += segmentList[i].chordLength;
  }

  StenoTokenizer *tokenizer = testSegments.CreateTokenizer();
  previousKeyCodeBuffer.Populate(tokenizer);
  delete tokenizer;

  char *lookup;

  if (testSegments[0].state->hasManualStateChange) {
    lookup = previousKeyCodeBuffer.ToString();
    chordThresholdCount++;
  } else {
    lookup = previousKeyCodeBuffer.ToUnresolvedString();
  }

  char *spaceRemoved = *lookup == ' ' ? lookup + 1 : lookup;

  testSegments.Reset();

  if (lastLookup) {
    if (*lastLookup == '\0' || Str::Eq(lookup, lastLookup)) {
      free(lastLookup);
      return lookup;
    }
    free(lastLookup);
  }

  // No need to check if there's a single chord producing the output.
  if (startSegmentIndex != segmentList.GetCount() - 1 ||
      chordThresholdCount != 1) {

    PrintPaperTapeSuggestion(spaceRemoved, chordThresholdCount, buffer,
                             chordThresholdCount);
  }

  return lookup;
}

//---------------------------------------------------------------------------
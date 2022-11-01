//---------------------------------------------------------------------------

#include "engine.h"
#include "thread.h"

//---------------------------------------------------------------------------

#if JAVELIN_THREADS

struct StenoEngine::UpdateNormalModeTextBufferThreadData {
  StenoEngine *engine;
  size_t maximumChordCount;
  StenoKeyCodeBuffer *buffer;
  size_t chordLength;
  size_t result;

  static void EntryPoint(void *data);
};

void StenoEngine::UpdateNormalModeTextBufferThreadData::EntryPoint(void *data) {
  UpdateNormalModeTextBufferThreadData *threadData =
      (UpdateNormalModeTextBufferThreadData *)data;

  threadData->result = threadData->engine->UpdateNormalModeTextBuffer(
      threadData->maximumChordCount, *threadData->buffer,
      threadData->chordLength);
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

  size_t previousSegmentCount = threadData[0].result;
  size_t nextSegmentCount = threadData[1].result;
#else
  size_t previousSegmentCount = UpdateNormalModeTextBuffer(
      history.GetCount(), previousKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT - 1);

  history.Add(chord, state);
  size_t nextSegmentCount = UpdateNormalModeTextBuffer(
      history.GetCount(), nextKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT);
#endif

  state = nextKeyCodeBuffer.state;

  if (nextKeyCodeBuffer.addTranslationCount >
      previousKeyCodeBuffer.addTranslationCount) {
    history.Pop();
    InitiateAddTranslationMode();
    return;
  }

  if (!emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer) &&
      nextSegmentCount > previousSegmentCount) {
    history.Pop();
  }
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
#else
  UpdateNormalModeTextBuffer(history.GetCount(), previousKeyCodeBuffer,
                             SEGMENT_CONVERSION_LIMIT);
  state = history.BackState();
  history.Pop();
  UpdateNormalModeTextBuffer(history.GetCount(),

                             nextKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT - 1);
#endif

  emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer);
}

size_t StenoEngine::UpdateNormalModeTextBuffer(size_t maximumChordCount,
                                               StenoKeyCodeBuffer &buffer,
                                               size_t chordLength) {
  StenoSegmentList segmentList =
      history.CreateSegments(maximumChordCount, dictionary, chordLength);
  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.Populate(tokenizer, *this);
  delete tokenizer;
  return segmentList.GetCount();
}

//---------------------------------------------------------------------------

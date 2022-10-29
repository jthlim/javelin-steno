//---------------------------------------------------------------------------

#include "engine.h"

//---------------------------------------------------------------------------
// cSpell:ignore TKUPT
//---------------------------------------------------------------------------

void StenoEngine::ProcessNormalModeChord(StenoChord chord) {
  history.ShiftIfFull();
  size_t previousSegmentCount = UpdateNormalModeTextBuffer(
      previousKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT - 1);

  history.Add(chord, state);
  size_t nextSegmentCount =
      UpdateNormalModeTextBuffer(nextKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT);
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

  UpdateNormalModeTextBuffer(previousKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT);
  state = history.BackState();
  history.Pop();
  UpdateNormalModeTextBuffer(nextKeyCodeBuffer, SEGMENT_CONVERSION_LIMIT - 1);

  emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer);
}

size_t StenoEngine::UpdateNormalModeTextBuffer(StenoKeyCodeBuffer &buffer,
                                               size_t chordLength) {
  StenoSegmentList segmentList =
      history.CreateSegments(dictionary, chordLength);
  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.Populate(tokenizer, *this);
  delete tokenizer;
  return segmentList.GetCount();
}

//---------------------------------------------------------------------------

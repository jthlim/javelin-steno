//---------------------------------------------------------------------------

#include "dictionary/dictionary.h"
#include "dictionary/user_dictionary.h"
#include "engine.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"

//---------------------------------------------------------------------------

static const char ADD_TRANSLATION_PROMPT[] = " >>> Add/Delete Translation - "
                                             "Chords: ";

static const char TRANSLATION_PROMPT[] = "; Translation: ";

const size_t MAX_USER_DICTIONARY_STROKE_COUNT = 10;

//---------------------------------------------------------------------------

void StenoEngine::InitiateAddTranslationMode() {
  mode = StenoEngineMode::ADD_TRANSLATION;

  addTranslationHistory.Reset();
  addTranslationState = state;
  addTranslationState.joinNext = true;

  previousKeyCodeBuffer.Reset();
  UpdateAddTranslationModeTextBuffer(nextKeyCodeBuffer);
  emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer);
}

void StenoEngine::ProcessAddTranslationModeChord(StenoChord chord) {
  size_t newlineIndex = 0;
  for (size_t i = 0; i < addTranslationHistory.GetCount(); ++i) {
    if (IsNewline(addTranslationHistory.GetChord(i))) {
      newlineIndex = i;
      break;
    }
  }

  size_t previousSegmentCount =
      UpdateAddTranslationModeTextBuffer(previousKeyCodeBuffer);

  if (IsNewline(chord)) {
    // Don't do anything with an empty chord.
    if (addTranslationHistory.IsEmpty()) {
      return;
    }
    if (newlineIndex != 0) {
      // Already one enter.
      if (newlineIndex + 1 == addTranslationHistory.GetCount()) {
        DeleteTranslation(newlineIndex);
      } else {
        AddTranslation(newlineIndex);
      }
      EndAddTranslationMode();
      return;
    }
  } else if (newlineIndex == 0) {
    if (addTranslationHistory.GetCount() > MAX_USER_DICTIONARY_STROKE_COUNT) {
      return;
    }

  } else if (newlineIndex + 1 == addTranslationHistory.GetCount()) {
    addTranslationState.isGlue = true;
    addTranslationState.joinNext = true;
  }

  if (addTranslationHistory.IsFull()) {
    return;
  }

  addTranslationHistory.Add(chord, addTranslationState);

  size_t nextSegmentCount =
      UpdateAddTranslationModeTextBuffer(nextKeyCodeBuffer);
  addTranslationState = nextKeyCodeBuffer.state;

  if (!emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer) &&
      nextSegmentCount > previousSegmentCount) {
    addTranslationHistory.Pop();
  }
}

void StenoEngine::ProcessAddTranslationModeUndo() {
  if (addTranslationHistory.IsEmpty()) {
    EndAddTranslationMode();
    return;
  }

  UpdateAddTranslationModeTextBuffer(previousKeyCodeBuffer);
  state = addTranslationHistory.BackState();
  addTranslationHistory.Pop();
  UpdateAddTranslationModeTextBuffer(nextKeyCodeBuffer);

  emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer);
}

size_t
StenoEngine::UpdateAddTranslationModeTextBuffer(StenoKeyCodeBuffer &buffer) {
  buffer.Reset();
  buffer.AppendText(ADD_TRANSLATION_PROMPT, sizeof(ADD_TRANSLATION_PROMPT) - 1,
                    StenoCaseMode::NORMAL);

  size_t i = 0;
  for (;;) {
    if (i >= addTranslationHistory.GetCount()) {
      return i;
    }
    StenoChord chord = addTranslationHistory.GetChord(i++);
    if (IsNewline(chord)) {
      break;
    }

    if (i != 1) {
      buffer.AppendText("/", 1, StenoCaseMode::NORMAL);
    }

    char chordBuffer[32];
    char *p = chord.ToString(chordBuffer);
    buffer.AppendText(chordBuffer, p - chordBuffer, StenoCaseMode::NORMAL);
  }

  buffer.AppendText(TRANSLATION_PROMPT, sizeof(TRANSLATION_PROMPT) - 1,
                    StenoCaseMode::NORMAL);

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, dictionary,
                              dictionary.GetMaximumMatchLength(), orthography,
                              addTranslationHistory.GetCount());

  addTranslationHistory.CreateSegments(context, 256, i);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.Append(tokenizer, orthography);
  delete tokenizer;
  return i + segmentList.GetCount();
}

void StenoEngine::EndAddTranslationMode() {
  UpdateAddTranslationModeTextBuffer(previousKeyCodeBuffer);
  nextKeyCodeBuffer.Reset();
  emitter.Process(previousKeyCodeBuffer, nextKeyCodeBuffer);

  mode = StenoEngineMode::NORMAL;
}

void StenoEngine::AddTranslation(size_t newlineIndex) {
  if (!userDictionary) {
    return;
  }

  nextKeyCodeBuffer.Reset();

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, dictionary,
                              dictionary.GetMaximumMatchLength(), orthography,
                              addTranslationHistory.GetCount());

  addTranslationHistory.CreateSegments(context, 256, newlineIndex + 1);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  nextKeyCodeBuffer.Append(tokenizer, orthography);
  delete tokenizer;

  char *word = nextKeyCodeBuffer.ToString();
  userDictionary->Add(&addTranslationHistory.GetChord(0), newlineIndex, word);
  free(word);
}

void StenoEngine::DeleteTranslation(size_t newlineIndex) {
  if (!userDictionary) {
    return;
  }

  userDictionary->Remove(&addTranslationHistory.GetChord(0), newlineIndex);
}

//---------------------------------------------------------------------------

bool StenoEngine::IsNewline(StenoChord chord) const {
  StenoDictionaryLookup lookup = dictionary.Lookup(&chord, 1);
  bool result = lookup.IsValid() && strchr(lookup.GetText(), '\n') != nullptr;
  lookup.Destroy();
  return result;
}

//---------------------------------------------------------------------------

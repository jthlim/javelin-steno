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

  previousConversionBuffer.keyCodeBuffer.Reset();
  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::ProcessAddTranslationModeChord(StenoChord chord) {
  size_t newlineIndex = 0;
  for (size_t i = 0; i < addTranslationHistory.GetCount(); ++i) {
    if (IsNewline(addTranslationHistory.GetChord(i))) {
      newlineIndex = i;
      break;
    }
  }

  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);

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

  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);
  addTranslationState = nextConversionBuffer.keyCodeBuffer.state;

  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    addTranslationHistory.SetBackCombineUndo();
  }
}

void StenoEngine::ProcessAddTranslationModeUndo() {
  if (addTranslationHistory.IsEmpty()) {
    EndAddTranslationMode();
    return;
  }

  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);

  size_t undoCount =
      addTranslationHistory.GetUndoCount(ChordHistory::BUFFER_SIZE);
  state = addTranslationHistory.BackState(undoCount);
  state.shouldCombineUndo = false;
  addTranslationHistory.PopCount(undoCount);

  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

size_t
StenoEngine::UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer) {
  buffer.keyCodeBuffer.Reset();
  buffer.keyCodeBuffer.AppendText(ADD_TRANSLATION_PROMPT,
                                  sizeof(ADD_TRANSLATION_PROMPT) - 1,
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
      buffer.keyCodeBuffer.AppendText("/", 1, StenoCaseMode::NORMAL);
    }

    char chordBuffer[32];
    char *p = chord.ToString(chordBuffer);
    buffer.keyCodeBuffer.AppendText(chordBuffer, p - chordBuffer,
                                    StenoCaseMode::NORMAL);
  }

  buffer.keyCodeBuffer.AppendText(TRANSLATION_PROMPT,
                                  sizeof(TRANSLATION_PROMPT) - 1,
                                  StenoCaseMode::NORMAL);

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, dictionary, orthography);

  buffer.chordHistory.TransferFrom(addTranslationHistory,
                                   addTranslationHistory.GetCount(),
                                   ChordHistory::BUFFER_SIZE);
  buffer.chordHistory.CreateSegments(context, i);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;
  return i + segmentList.GetCount();
}

void StenoEngine::EndAddTranslationMode() {
  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);
  nextConversionBuffer.keyCodeBuffer.Reset();
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  mode = StenoEngineMode::NORMAL;
}

void StenoEngine::AddTranslation(size_t newlineIndex) {
  if (!userDictionary) {
    return;
  }

  nextConversionBuffer.keyCodeBuffer.Reset();

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, dictionary, orthography);

  nextConversionBuffer.chordHistory.TransferFrom(
      addTranslationHistory, addTranslationHistory.GetCount(),
      ChordHistory::BUFFER_SIZE);
  nextConversionBuffer.chordHistory.CreateSegments(context, newlineIndex + 1);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  nextConversionBuffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;

  char *word = nextConversionBuffer.keyCodeBuffer.ToString();
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
  StenoDictionaryLookupResult lookup = dictionary.Lookup(&chord, 1);

  bool result =
      lookup.IsValid() && (strchr(lookup.GetText(), '\n') != nullptr ||
                           strstr(lookup.GetText(), "{#Return}") != nullptr);
  lookup.Destroy();
  return result;
}

//---------------------------------------------------------------------------

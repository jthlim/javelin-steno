//---------------------------------------------------------------------------

#include "dictionary/dictionary.h"
#include "dictionary/unicode_dictionary.h"
#include "dictionary/user_dictionary.h"
#include "engine.h"
#include "key_code.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"

//---------------------------------------------------------------------------

static const char ADD_TRANSLATION_PROMPT[] = " >>> Add/Delete Translation - "
                                             "Strokes: ";

static const char TRANSLATION_PROMPT[] = "; Translation: ";

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

void StenoEngine::ProcessAddTranslationModeStroke(StenoStroke stroke) {
  size_t newlineIndex = 0;
  for (size_t i = 0; i < addTranslationHistory.GetCount(); ++i) {
    if (IsNewline(addTranslationHistory[i].stroke)) {
      newlineIndex = i;
      break;
    }
  }

  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);

  if (IsNewline(stroke)) {
    // Don't do anything with an empty stroke.
    if (addTranslationHistory.IsEmpty()) {
      return;
    }
    if (newlineIndex != 0) {
      // Already one enter -- this is the second
      if (newlineIndex + 1 == addTranslationHistory.GetCount()) {
        DeleteTranslation(newlineIndex);
      } else {
        AddTranslation(newlineIndex);
      }
      EndAddTranslationMode();
      return;
    }
  } else if (newlineIndex == 0) {
    // No newline yet -- record stroke input.
    if (addTranslationHistory.GetCount() >
        StenoUserDictionary::MAX_STROKE_COUNT) {
      return;
    }
    if ((stroke & StrokeMask::UNICODE).IsNotEmpty()) {
      // Don't allow unicode when defining the stroke.
      return;
    }
  } else if (newlineIndex + 1 == addTranslationHistory.GetCount()) {
    addTranslationState.isGlue = true;
    addTranslationState.joinNext = true;
  }

  if (addTranslationHistory.IsFull()) {
    return;
  }

  addTranslationHistory.Add(stroke, addTranslationState);

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
      addTranslationHistory.GetUndoCount(StenoSegmentBuilder::BUFFER_SIZE);
  state = addTranslationHistory.Back(undoCount).state;
  state.shouldCombineUndo = false;
  addTranslationHistory.RemoveBack(undoCount);

  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

size_t
StenoEngine::UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer) {
  buffer.keyCodeBuffer.Reset();
  buffer.keyCodeBuffer.AppendText(ADD_TRANSLATION_PROMPT,
                                  Str::Length<>(ADD_TRANSLATION_PROMPT),
                                  StenoCaseMode::NORMAL);

  size_t i = 0;
  for (;;) {
    if (i >= addTranslationHistory.GetCount()) {
      return i;
    }
    StenoStroke stroke = addTranslationHistory[i++].stroke;
    if (IsNewline(stroke)) {
      break;
    }

    if (i != 1) {
      buffer.keyCodeBuffer.AppendText("/", 1, StenoCaseMode::NORMAL);
    }

    char strokeBuffer[StenoStroke::MAX_STRING_LENGTH];
    char *p = stroke.ToString(strokeBuffer);
    buffer.keyCodeBuffer.AppendText(strokeBuffer, p - strokeBuffer,
                                    StenoCaseMode::NORMAL);
  }

  buffer.keyCodeBuffer.AppendText(TRANSLATION_PROMPT,
                                  Str::Length<>(TRANSLATION_PROMPT),
                                  StenoCaseMode::NORMAL);

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, dictionary, orthography);

  buffer.segmentBuilder.TransferFrom(addTranslationHistory,
                                     addTranslationHistory.GetCount(),
                                     StenoSegmentBuilder::BUFFER_SIZE);
  buffer.segmentBuilder.CreateSegments(context, i);
  addTranslationHistory.UpdateDefinitionBoundaries(i, segmentList);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  buffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;
  if (placeSpaceAfter && !buffer.keyCodeBuffer.state.joinNext &&
      buffer.segmentBuilder.IsNotEmpty()) {
    buffer.keyCodeBuffer.AppendSpace();
  }
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

  nextConversionBuffer.segmentBuilder.TransferFrom(
      addTranslationHistory, addTranslationHistory.GetCount(),
      StenoSegmentBuilder::BUFFER_SIZE);
  nextConversionBuffer.segmentBuilder.CreateSegments(context, newlineIndex + 1);

  StenoTokenizer *tokenizer = segmentList.CreateTokenizer();
  nextConversionBuffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;

  char *word = nextConversionBuffer.keyCodeBuffer.ToString();
  StenoStroke strokes[newlineIndex];
  for (size_t i = 0; i < newlineIndex; ++i) {
    strokes[i] = addTranslationHistory[i].stroke;
  }
  userDictionary->Add(strokes, newlineIndex, word);
  free(word);
}

void StenoEngine::DeleteTranslation(size_t newlineIndex) {
  if (!userDictionary) {
    return;
  }

  StenoStroke strokes[newlineIndex];
  for (size_t i = 0; i < newlineIndex; ++i) {
    strokes[i] = addTranslationHistory[i].stroke;
  }
  userDictionary->Remove(strokes, newlineIndex);
}

//---------------------------------------------------------------------------

bool StenoEngine::HandleAddTranslationModeScanCode(
    uint32_t scanCodeAndModifiers, ScanCodeAction action) {
  KeyCode keyCode = KeyCode::Value(scanCodeAndModifiers & 0xff);
  if (keyCode.IsModifier()) {
    return false;
  }

  if (action == ScanCodeAction::PRESS || action == ScanCodeAction::TAP) {
    uint32_t unicode = KeyCode::ConvertToUnicode(scanCodeAndModifiers);
    if (unicode == '\b') {
      ProcessAddTranslationModeUndo();
    } else if (unicode != 0) {
      const StenoStroke unicodeStroke =
          StenoUnicodeDictionary::CreateUnicodeStroke(unicode);
      ProcessAddTranslationModeStroke(unicodeStroke);
    }
  }
  return true;
}

//---------------------------------------------------------------------------

bool StenoEngine::IsNewline(StenoStroke stroke) const {
  if (stroke == StenoStroke(StrokeMask::LR | StrokeMask::RR)) {
    return true;
  }
  if (stroke == StenoStroke(StrokeMask::UNICODE | '\n')) {
    return true;
  }

  StenoDictionaryLookupResult lookup = dictionary.Lookup(&stroke, 1);
  if (!lookup.IsValid()) {
    return false;
  }

  bool result = (strchr(lookup.GetText(), '\n') != nullptr ||
                 Str::Eq(lookup.GetText(), "{#Return}"));
  lookup.Destroy();
  return result;
}

//---------------------------------------------------------------------------

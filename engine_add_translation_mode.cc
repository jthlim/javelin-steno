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
  if (!userDictionary) {
    return;
  }

  mode = StenoEngineMode::ADD_TRANSLATION;

  altTranslationHistory.Reset();
  altTranslationState = state;
  altTranslationState.joinNext = true;

  previousConversionBuffer.keyCodeBuffer.Reset();
  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::ProcessAddTranslationModeStroke(StenoStroke stroke) {
  size_t newlineIndex = 0;
  for (size_t i = 0; i < altTranslationHistory.GetCount(); ++i) {
    if (IsNewline(altTranslationHistory[i].stroke)) {
      newlineIndex = i;
      break;
    }
  }

  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);

  if (IsNewline(stroke)) {
    // Don't do anything with an empty stroke.
    if (altTranslationHistory.IsEmpty()) {
      return;
    }
    if (newlineIndex != 0) {
      // Already one enter -- this is the second
      if (newlineIndex + 1 == altTranslationHistory.GetCount()) {
        DeleteTranslation(newlineIndex);
      } else {
        AddTranslation(newlineIndex);
      }
      EndAddTranslationMode();
      return;
    }
  } else if (newlineIndex == 0) {
    // No newline yet -- record stroke input.
    if (altTranslationHistory.GetCount() >
        StenoUserDictionary::MAX_STROKE_COUNT) {
      return;
    }
    if ((stroke & StrokeMask::UNICODE).IsNotEmpty()) {
      // Don't allow unicode when defining the stroke.
      return;
    }
  } else if (newlineIndex + 1 == altTranslationHistory.GetCount()) {
    altTranslationState.isGlue = true;
    altTranslationState.joinNext = true;
  }

  if (altTranslationHistory.IsFull()) {
    return;
  }

  altTranslationHistory.Add(stroke, altTranslationState);

  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);
  altTranslationState = nextConversionBuffer.keyCodeBuffer.state;

  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    altTranslationHistory.SetBackCombineUndo();
  }
}

void StenoEngine::ProcessAddTranslationModeUndo() {
  if (altTranslationHistory.IsEmpty()) {
    EndAddTranslationMode();
    return;
  }

  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);

  const size_t undoCount =
      altTranslationHistory.GetUndoCount(StenoSegmentBuilder::BUFFER_SIZE);
  altTranslationState = altTranslationHistory.Back(undoCount).state;
  altTranslationState.shouldCombineUndo = false;
  altTranslationHistory.RemoveBack(undoCount);

  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer) {
  buffer.keyCodeBuffer.Reset();
  buffer.keyCodeBuffer.AppendText(ADD_TRANSLATION_PROMPT,
                                  Str::Length<>(ADD_TRANSLATION_PROMPT),
                                  StenoCaseMode::NORMAL);

  size_t i = 0;
  for (;;) {
    if (i >= altTranslationHistory.GetCount()) {
      return;
    }
    const StenoStroke stroke = altTranslationHistory[i++].stroke;
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

  buffer.segmentBuilder.TransferFrom(altTranslationHistory,
                                     altTranslationHistory.GetCount(),
                                     StenoSegmentBuilder::BUFFER_SIZE);
  buffer.segmentBuilder.CreateSegments(context, i);
  altTranslationHistory.UpdateDefinitionBoundaries(i, segmentList);

  StenoTokenizer *tokenizer = StenoTokenizer::Create(segmentList);
  buffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;
  if (placeSpaceAfter && !buffer.keyCodeBuffer.state.joinNext &&
      buffer.segmentBuilder.IsNotEmpty()) {
    buffer.keyCodeBuffer.AppendSpace();
  }
}

void StenoEngine::EndAddTranslationMode() {
  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);
  nextConversionBuffer.keyCodeBuffer.Reset();
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  mode = StenoEngineMode::NORMAL;
  ProcessNormalModeUndo();
}

void StenoEngine::AddTranslation(size_t newlineIndex) {
  if (!userDictionary) {
    return;
  }

  nextConversionBuffer.keyCodeBuffer.Reset();

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, dictionary, orthography);

  nextConversionBuffer.segmentBuilder.TransferFrom(
      altTranslationHistory, altTranslationHistory.GetCount(),
      StenoSegmentBuilder::BUFFER_SIZE);
  nextConversionBuffer.segmentBuilder.CreateSegments(context, newlineIndex + 1);

  StenoTokenizer *tokenizer = StenoTokenizer::Create(segmentList);
  nextConversionBuffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;

  char *word = nextConversionBuffer.keyCodeBuffer.ToString();
  StenoStroke strokes[newlineIndex];
  for (size_t i = 0; i < newlineIndex; ++i) {
    strokes[i] = altTranslationHistory[i].stroke;
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
    strokes[i] = altTranslationHistory[i].stroke;
  }
  userDictionary->Remove(strokes, newlineIndex);
}

//---------------------------------------------------------------------------

bool StenoEngine::HandleAddTranslationModeScanCode(
    uint32_t scanCodeAndModifiers, ScanCodeAction action) {
  const KeyCode keyCode = KeyCode::Value(scanCodeAndModifiers & 0xff);
  if (keyCode.IsModifier()) {
    return false;
  }

  if (action == ScanCodeAction::PRESS || action == ScanCodeAction::TAP) {
    const uint32_t unicode = KeyCode::ConvertToUnicode(scanCodeAndModifiers);
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
  // Special case R-R from plover theory.
  if (stroke == StenoStroke(StrokeMask::LR | StrokeMask::RR)) {
    return true;
  }

  // Handle 'enter' in qwerty mode press.
  if (stroke == StenoStroke(StrokeMask::UNICODE | '\n')) {
    return true;
  }

  // Standalone newlines are treated as delimiters.
  StenoDictionaryLookupResult lookup = dictionary.Lookup(&stroke, 1);
  if (!lookup.IsValid()) {
    return false;
  }

  static const char *const VALID_NEWLINES[] = {
      "{#Return}",
      "{#Return}{^}",
      "{^\n^}",
      "{^~|\n^}",
  };

  const char *text = lookup.GetText();
  bool result = false;
  for (const char *validNewline : VALID_NEWLINES) {
    if (Str::Eq(text, validNewline)) {
      result = true;
      break;
    }
  }

  lookup.Destroy();
  return result;
}

//---------------------------------------------------------------------------

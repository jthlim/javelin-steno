//---------------------------------------------------------------------------

#include "dictionary/dictionary.h"
#include "dictionary/unicode_dictionary.h"
#include "dictionary/user_dictionary.h"
#include "engine.h"
#include "key_code.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"

//---------------------------------------------------------------------------

static constexpr char ADD_TRANSLATION_PROMPT[] = " >>> Add Translation - "
                                                 "Strokes: ";

static constexpr char ADD_DELETE_TRANSLATION_PROMPT[] =
    " >>> Add/Delete Translation - "
    "Strokes: ";

static constexpr char TRANSLATION_PROMPT[] = "; Translation: ";

//---------------------------------------------------------------------------

void StenoEngine::FreeAddTranslationText() {
  free(addTranslationText);
  addTranslationText = nullptr;
}

void StenoEngine::InitiateAddTranslationMode(const char *text) {
  if (!userDictionary) {
    return;
  }

  FreeAddTranslationText();
  if (text) {
    addTranslationText = Str::Dup(text);
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
    if (addTranslationText) {
      AddTranslation(altTranslationHistory.GetCount());
      EndAddTranslationMode();
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
  altTranslationState = nextConversionBuffer.keyCodeBuffer.GetPersistentState();

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

  const size_t undoCount = altTranslationHistory.GetUndoCount();
  altTranslationState =
      altTranslationHistory.Back(undoCount).state.GetPersistentState();
  altTranslationHistory.RemoveBack(undoCount);

  UpdateAddTranslationModeTextBuffer(nextConversionBuffer);

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::UpdateAddTranslationModeTextBuffer(ConversionBuffer &buffer) {
  buffer.keyCodeBuffer.Reset();

  if (addTranslationText) {
    buffer.keyCodeBuffer.AppendText(ADD_TRANSLATION_PROMPT,
                                    Str::ConstLength(ADD_TRANSLATION_PROMPT),
                                    StenoCaseMode::NORMAL);
  } else {
    buffer.keyCodeBuffer.AppendText(
        ADD_DELETE_TRANSLATION_PROMPT,
        Str::ConstLength(ADD_DELETE_TRANSLATION_PROMPT), StenoCaseMode::NORMAL);
  }

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
                                  Str::ConstLength(TRANSLATION_PROMPT),
                                  StenoCaseMode::NORMAL);

  StenoSegmentList segments(altTranslationHistory.GetCount());
  BuildSegmentContext context(segments, *this);

  buffer.segmentBuilder.TransferFrom(altTranslationHistory,
                                     altTranslationHistory.GetCount(),
                                     StenoSegmentBuilder::BUFFER_SIZE);
  buffer.segmentBuilder.CreateSegments(context, i);
  altTranslationHistory.UpdateDefinitionBoundaries(
      i, segments, buffer.segmentBuilder.GetStrokes(0));

  StenoTokenizer tokenizer(segments);
  buffer.keyCodeBuffer.Append(tokenizer);
  if (placeSpaceAfter && segments.IsNotEmpty()) {
    const StenoState lastState = buffer.keyCodeBuffer.state;
    if (!lastState.joinNext) {
      buffer.keyCodeBuffer.AppendSpace();
    }
  }
}

void StenoEngine::EndAddTranslationMode() {
  UpdateAddTranslationModeTextBuffer(previousConversionBuffer);
  nextConversionBuffer.keyCodeBuffer.Reset();
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  mode = StenoEngineMode::NORMAL;
  ProcessNormalModeUndo();

  FreeAddTranslationText();
}

void StenoEngine::AddTranslation(size_t newlineIndex) {
  if (!userDictionary) {
    return;
  }

  char *word;

  if (addTranslationText) {
    word = addTranslationText;
  } else {
    nextConversionBuffer.keyCodeBuffer.Reset();

    StenoSegmentList segments(altTranslationHistory.GetCount());
    BuildSegmentContext context(segments, *this);

    nextConversionBuffer.segmentBuilder.TransferFrom(
        altTranslationHistory, altTranslationHistory.GetCount(),
        StenoSegmentBuilder::BUFFER_SIZE);
    nextConversionBuffer.segmentBuilder.CreateSegments(context,
                                                       newlineIndex + 1);

    StenoTokenizer tokenizer(segments);
    nextConversionBuffer.keyCodeBuffer.Append(tokenizer);

    word = nextConversionBuffer.keyCodeBuffer.ToString();
  }

  StenoStroke strokes[newlineIndex];
  for (size_t i = 0; i < newlineIndex; ++i) {
    strokes[i] = altTranslationHistory[i].stroke;
  }
  userDictionary->Add(strokes, newlineIndex, word);
  if (word != addTranslationText) {
    free(word);
  }
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
  // Handle 'enter' in qwerty mode press.
  if (stroke == StenoStroke(StrokeMask::UNICODE | '\n') ||
      stroke == StenoStroke(StrokeMask::UNICODE | '\r')) {
    return true;
  }

  // Standalone newlines are treated as delimiters.
  StenoDictionaryLookupResult lookup = GetDictionary().Lookup(&stroke, 1);
  if (!lookup.IsValid()) {
    return false;
  }

  static constexpr const char *VALID_NEWLINES[] = {
      "{#Return}", "{#Return}{^}",                         //
      "\n",        "{^}\n{^}",     "{^\n^}",  "{^~|\n^}",  //
      "\\n",       "{^}\\n{^}",    "{^\\n^}", "{^~|\\n^}", //
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

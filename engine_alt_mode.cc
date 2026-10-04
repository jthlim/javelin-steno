//---------------------------------------------------------------------------

#include "console.h"
#include "dictionary/unicode_dictionary.h"
#include "engine.h"
#include "key_code.h"
#include "steno_key_code_emitter_hid_writer.h"

//---------------------------------------------------------------------------

void StenoEngine::InitiateAltMode(StenoEngineMode newMode) {
  mode = newMode;

  altTranslationHistory.Reset();
  altTranslationState = state;
  altTranslationState.joinNext = true;

  previousConversionBuffer.keyCodeBuffer.Reset();
  UpdateAltModeTextBuffer(nextConversionBuffer);
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::ProcessAltModeStroke(StenoStroke stroke) {
  UpdateAltModeTextBuffer(previousConversionBuffer);

  if (IsNewline(stroke)) {
    if (altTranslationHistory.IsEmpty()) {
      EndAltMode();
      return;
    }
    switch (mode) {
    case StenoEngineMode::NORMAL:
    case StenoEngineMode::ADD_TRANSLATION:
      break;
    case StenoEngineMode::CONSOLE:
      ConsoleModeExecute();
      break;
    case StenoEngineMode::LOOKUP:
      LookupModeExecute();
      break;
    }
    mode = StenoEngineMode::NORMAL;
    ResetState();
    return;
  }

  if (altTranslationHistory.IsFull()) {
    return;
  }

  altTranslationHistory.Add(stroke, altTranslationState);

  UpdateAltModeTextBuffer(nextConversionBuffer);
  altTranslationState = nextConversionBuffer.keyCodeBuffer.GetPersistentState();

  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    altTranslationHistory.SetBackCombineUndo();
  }
}

void StenoEngine::ProcessAltModeUndo() {
  if (altTranslationHistory.IsEmpty()) {
    EndAltMode();
    return;
  }

  UpdateAltModeTextBuffer(previousConversionBuffer);

  const size_t undoCount = altTranslationHistory.GetUndoCount();
  altTranslationState =
      altTranslationHistory.Back(undoCount).state.GetPersistentState();
  altTranslationHistory.RemoveBack(undoCount);

  UpdateAltModeTextBuffer(nextConversionBuffer);

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::UpdateAltModeTextBuffer(ConversionBuffer &buffer) {
  buffer.keyCodeBuffer.Reset();

  StenoSegmentList segments(altTranslationHistory.GetCount());
  BuildSegmentContext context(segments, *this);

  buffer.segmentBuilder.TransferFrom(altTranslationHistory,
                                     altTranslationHistory.GetCount(),
                                     StenoSegmentBuilder::BUFFER_SIZE);
  buffer.segmentBuilder.CreateSegments(context);
  altTranslationHistory.UpdateDefinitionBoundaries(
      0, segments, buffer.segmentBuilder.GetStrokes(0));

  StenoTokenizer tokenizer(segments, 0, 0);
  buffer.keyCodeBuffer.Append(tokenizer, false);
  if (placeSpaceAfter && segments.IsNotEmpty()) {
    const StenoState lastState = buffer.keyCodeBuffer.state;
    if (!lastState.joinNext) {
      buffer.keyCodeBuffer.AppendSpace();
    }
  }
}

void StenoEngine::EndAltMode() {
  UpdateAltModeTextBuffer(previousConversionBuffer);
  nextConversionBuffer.keyCodeBuffer.Reset();
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  mode = StenoEngineMode::NORMAL;
  ProcessNormalModeUndo();
}

//---------------------------------------------------------------------------

bool StenoEngine::HandleAltModeScanCode(uint32_t scanCodeAndModifiers,
                                        ScanCodeAction action) {
  const KeyCode keyCode = KeyCode::Value(scanCodeAndModifiers & 0xff);
  if (keyCode.IsModifier()) {
    return false;
  }

  if (action == ScanCodeAction::PRESS || action == ScanCodeAction::TAP) {
    const uint32_t unicode = KeyCode::ConvertToUnicode(scanCodeAndModifiers);
    if (unicode == '\b') {
      ProcessUndo();
    } else if (unicode != 0) {
      const StenoStroke unicodeStroke =
          StenoUnicodeDictionary::CreateUnicodeStroke(unicode);
      ProcessStroke(unicodeStroke);
    }
  }
  return true;
}

//---------------------------------------------------------------------------

void StenoEngine::ConsoleModeExecute() {
  char *command = previousConversionBuffer.keyCodeBuffer.ToString();
  StenoKeyCodeEmitter::HidWriter writer;
  writer.context.TapKey(KeyCode::ENTER);
  if (!Console::RunCommand(command, writer)) {
    writer.Printf(
        "ERR Invalid command. Use \"help\" for a list of commands\n\n");
  }
  free(command);
}

void StenoEngine::LookupModeExecute() {
  char *definition = previousConversionBuffer.keyCodeBuffer.ToString();
  StenoKeyCodeEmitter::HidWriter writer;

  StenoReverseDictionaryLookup lookup(definition);
  ReverseLookup(lookup);
  free(definition);

  if (!lookup.HasResults()) {
    writer.Printf("\nNo definitions found\n\n");
    return;
  }

  for (const StenoReverseDictionaryResult &entry : lookup.results) {
    StenoSegmentList segments(entry.length);
    ConversionBuffer &buffer = previousConversionBuffer;
    CreateSegments(segments, buffer.segmentBuilder, entry.strokes,
                   entry.length);

    // Print definition.
    if (segments.GetCount() == 1) {
      char *t = Str::Trim(segments[0].lookup.GetText());
      writer.Printf("\n%T: \"%J\"", entry.strokes, entry.length, t);
      free(t);
    } else {
      BufferWriter buffer;
      segments.WriteToBuffer(buffer);
      writer.Printf("\n%T: \"%J\"", entry.strokes, entry.length,
                    buffer.GetBuffer());
    }
    if (!entry.dictionary->IsInternal()) {
      writer.Printf(" (%J)", entry.dictionary->GetName());
    }
  }

  writer.Printf("\n\n");
}

//---------------------------------------------------------------------------

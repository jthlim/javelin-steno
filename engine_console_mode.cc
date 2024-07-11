//---------------------------------------------------------------------------

#include "console.h"
#include "dictionary/unicode_dictionary.h"
#include "engine.h"
#include "key_code.h"
#include "steno_key_code_emitter_context.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

void StenoEngine::InitiateConsoleMode() {
  mode = StenoEngineMode::CONSOLE;

  altTranslationHistory.Reset();
  altTranslationState = state;
  altTranslationState.joinNext = true;

  previousConversionBuffer.keyCodeBuffer.Reset();
  UpdateConsoleModeTextBuffer(nextConversionBuffer);
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::ProcessConsoleModeStroke(StenoStroke stroke) {
  UpdateConsoleModeTextBuffer(previousConversionBuffer);

  if (IsNewline(stroke)) {
    ConsoleModeExecute();
    mode = StenoEngineMode::NORMAL;
    ResetState();
    return;
  }

  if (altTranslationHistory.IsFull()) {
    return;
  }

  altTranslationHistory.Add(stroke, altTranslationState, 0);

  UpdateConsoleModeTextBuffer(nextConversionBuffer);
  altTranslationState = nextConversionBuffer.keyCodeBuffer.state;

  if (emitter.Process(previousConversionBuffer.keyCodeBuffer,
                      nextConversionBuffer.keyCodeBuffer)) {
    altTranslationHistory.SetBackCombineUndo();
  }
}

void StenoEngine::ProcessConsoleModeUndo() {
  if (altTranslationHistory.IsEmpty()) {
    EndConsoleMode();
    return;
  }

  UpdateConsoleModeTextBuffer(previousConversionBuffer);

  const size_t undoCount =
      altTranslationHistory.GetUndoCount(StenoSegmentBuilder::BUFFER_SIZE);
  altTranslationState = altTranslationHistory.Back(undoCount).state;
  altTranslationState.shouldCombineUndo = false;
  altTranslationHistory.RemoveBack(undoCount);

  UpdateConsoleModeTextBuffer(nextConversionBuffer);

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::UpdateConsoleModeTextBuffer(ConversionBuffer &buffer) {
  buffer.keyCodeBuffer.Reset();

  StenoSegmentList segmentList;
  BuildSegmentContext context(segmentList, *this, false);

  buffer.segmentBuilder.TransferFrom(altTranslationHistory,
                                     altTranslationHistory.GetCount(),
                                     StenoSegmentBuilder::BUFFER_SIZE);
  buffer.segmentBuilder.CreateSegments(context);
  altTranslationHistory.UpdateDefinitionBoundaries(0, segmentList);

  StenoTokenizer *tokenizer = StenoTokenizer::Create(segmentList);
  buffer.keyCodeBuffer.Append(tokenizer);
  delete tokenizer;
  if (placeSpaceAfter && !buffer.keyCodeBuffer.state.joinNext &&
      buffer.segmentBuilder.IsNotEmpty()) {
    buffer.keyCodeBuffer.AppendSpace();
  }
}

void StenoEngine::EndConsoleMode() {
  UpdateConsoleModeTextBuffer(previousConversionBuffer);
  nextConversionBuffer.keyCodeBuffer.Reset();
  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);

  mode = StenoEngineMode::NORMAL;
  ProcessNormalModeUndo();
}

//---------------------------------------------------------------------------

bool StenoEngine::HandleConsoleModeScanCode(uint32_t scanCodeAndModifiers,
                                            ScanCodeAction action) {
  const KeyCode keyCode = KeyCode::Value(scanCodeAndModifiers & 0xff);
  if (keyCode.IsModifier()) {
    return false;
  }

  if (action == ScanCodeAction::PRESS || action == ScanCodeAction::TAP) {
    const uint32_t unicode = KeyCode::ConvertToUnicode(scanCodeAndModifiers);
    if (unicode == '\b') {
      ProcessConsoleModeUndo();
    } else if (unicode != 0) {
      const StenoStroke unicodeStroke =
          StenoUnicodeDictionary::CreateUnicodeStroke(unicode);
      ProcessConsoleModeStroke(unicodeStroke);
    }
  }
  return true;
}

//---------------------------------------------------------------------------

class HidWriter final : public IWriter {
public:
  HidWriter() {}
  ~HidWriter() { context.ReleaseModifiers(context.modifiers); }

  virtual void Write(const char *data, size_t length);

  StenoKeyCodeEmitter::EmitterContext context;
};

void HidWriter::Write(const char *data, size_t length) {
  Utf8Pointer utf8(data);
  const char *end = data + length;

  while (utf8 < end) {
    const uint32_t unicode = *utf8++;

    const StenoKeyCode stenoKeyCode(unicode, StenoCaseMode::NORMAL);
    context.ProcessStenoKeyCode(stenoKeyCode);
  }
};

void StenoEngine::ConsoleModeExecute() {
  char *command = previousConversionBuffer.keyCodeBuffer.ToString();
  HidWriter writer;
  writer.context.TapKey(KeyCode::ENTER);
  if (!Console::RunCommand(command, writer)) {
    writer.Printf(
        "ERR Invalid command. Use \"help\" for a list of commands\n\n");
  }
  free(command);
}

//---------------------------------------------------------------------------

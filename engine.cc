//---------------------------------------------------------------------------

#include "engine.h"

#include "clock.h"
#include "console.h"
#include "dictionary/user_dictionary.h"
#include "flash.h"
#include "hal/external_flash.h"
#include "key.h"
#include "list.h"
#include "orthography.h"
#include "pattern.h"
#include "segment.h"
#include "steno_key_code_buffer.h"
#include "str.h"
#include "stroke.h"
#include "word_list.h"

//---------------------------------------------------------------------------

const StenoStroke StenoEngine::UNDO_STROKE(StrokeMask::STAR);

//---------------------------------------------------------------------------

StenoEngine::StenoEngine(StenoDictionary &dictionary,
                         const StenoCompiledOrthography &orthography,
                         StenoUserDictionary *userDictionary)
    : dictionary(dictionary), orthography(orthography),
      userDictionary(userDictionary) {

  previousConversionBuffer.Prepare(&this->orthography, &this->dictionary);
  nextConversionBuffer.Prepare(&this->orthography, &this->dictionary);
  ResetState();
}

//---------------------------------------------------------------------------

void StenoEngine::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }

  ++strokeCount;
  const StenoStroke stroke = value.ToStroke();
  if (stroke == UNDO_STROKE) {
    ProcessUndo();
  } else {
    ProcessStroke(stroke);
  }
}

void StenoEngine::ProcessStroke(StenoStroke stroke) {
  const ExternalFlashSentry externalFlashSentry;

  switch (mode) {
  case StenoEngineMode::NORMAL:
    ProcessNormalModeStroke(stroke);
    return;

  case StenoEngineMode::ADD_TRANSLATION:
    ProcessAddTranslationModeStroke(stroke);
    return;

  case StenoEngineMode::CONSOLE:
    ProcessConsoleModeStroke(stroke);
    return;
  }
}

void StenoEngine::ProcessUndo() {
  const ExternalFlashSentry externalFlashSentry;

  switch (mode) {
  case StenoEngineMode::NORMAL:
    ProcessNormalModeUndo();
    return;

  case StenoEngineMode::ADD_TRANSLATION:
    ProcessAddTranslationModeUndo();
    return;

  case StenoEngineMode::CONSOLE:
    ProcessConsoleModeUndo();
    return;
  }
}

bool StenoEngine::ProcessScanCode(uint32_t scanCodeAndModifiers,
                                  ScanCodeAction action) {
  const ExternalFlashSentry externalFlashSentry;

  switch (mode) {
  case StenoEngineMode::NORMAL:
    return false;

  case StenoEngineMode::ADD_TRANSLATION:
    return HandleAddTranslationModeScanCode(scanCodeAndModifiers, action);

  case StenoEngineMode::CONSOLE:
    return HandleConsoleModeScanCode(scanCodeAndModifiers, action);
  }
  return false;
}

//---------------------------------------------------------------------------

void StenoEngine::ResetState() {
  history.Reset();
  altTranslationHistory.Reset();
  state.Reset();
  state.joinNext = true;
}

//---------------------------------------------------------------------------

void StenoEngine::PrintInfo() const {
  Console::Printf("  Javelin Steno Engine\n");
  Console::Printf("    Strokes: %zu\n", strokeCount);
  Console::Printf("    Unicode mode: %s\n", emitter.GetUnicodeModeName());
  Console::Printf("    Keyboard layout: %s\n",
                  KeyboardLayout::GetActiveLayout().GetName());
  Console::Printf("    Space position: %s\n",
                  placeSpaceAfter ? "after" : "before");

  orthography.PrintInfo();

  const ExternalFlashSentry externalFlashSentry;

  Console::Printf("    Dictionaries\n");
  dictionary.PrintInfo(4);
}

void StenoEngine::PrintDictionary(const char *name) const {
  const ExternalFlashSentry externalFlashSentry;

  Console::Printf("{");
  PrintDictionaryContext context(name);
  dictionary.PrintDictionary(context);
  Console::Printf("\n}\n\n");
}

void StenoEngine::ListDictionaries() const {
  const ExternalFlashSentry externalFlashSentry;
  dictionary.ListDictionaries();
}

bool StenoEngine::EnableDictionary(const char *name) {
  const ExternalFlashSentry externalFlashSentry;
  return dictionary.EnableDictionary(name);
}

bool StenoEngine::DisableDictionary(const char *name) {
  const ExternalFlashSentry externalFlashSentry;
  return dictionary.DisableDictionary(name);
}

bool StenoEngine::ToggleDictionary(const char *name) {
  const ExternalFlashSentry externalFlashSentry;
  return dictionary.ToggleDictionary(name);
}

void StenoEngine::ReverseLookup(StenoReverseDictionaryLookup &lookup) const {
  const ExternalFlashSentry externalFlashSentry;

  dictionary.ReverseLookup(lookup);
  if (lookup.results.IsEmpty()) {
    return;
  }

  lookup.SortResults();
}

void StenoEngine::SendText(const uint8_t *p) {
  const char *ccp = (const char *)p;

  nextConversionBuffer.keyCodeBuffer.Reset();
  nextConversionBuffer.keyCodeBuffer.AppendTextNoCaseModeOverride(
      ccp, strlen(ccp), StenoCaseMode::NORMAL);
  previousConversionBuffer.keyCodeBuffer.Reset();

  emitter.Process(previousConversionBuffer.keyCodeBuffer,
                  nextConversionBuffer.keyCodeBuffer);
}

__attribute__((weak)) void StenoEngine::Pump() {}

//---------------------------------------------------------------------------

#include "key_code.h"
#include "unit_test.h"
#include <stdio.h>

#include "dictionary/compact_map_dictionary.h"
#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/jeff_numbers_dictionary.h"
#include "dictionary/jeff_phrasing_dictionary.h"
#include "dictionary/jeff_show_stroke_dictionary.h"
#include "dictionary/test_dictionary.h"
#include "dictionary/unicode_dictionary.h"

extern StenoOrthography testOrthography;

// extern StenoCompactMapDictionaryDefinition testDictionaryDefinition;
// static StenoCompactMapDictionary mainDictionary(testDictionaryDefinition);
static StenoCompactMapDictionary mainDictionary(TestDictionary::definition);

static StenoCompactMapDictionary testDictionary(TestDictionary::definition);

StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::instance,
    &mainDictionary,
};

class StenoEngineTester {
public:
  static void TestSymbols(StenoEngine &engine);
  static void TestEngine(StenoEngine &engine);
  static void TestAddTranslation(StenoEngine &engine);
  static void TestScancodeAddTranslation(StenoEngine &engine);
  static void VerifyTextBuffer(StenoEngine &engine, const char *expected);
};

void StenoEngineTester::VerifyTextBuffer(StenoEngine &engine,
                                         const char *expected) {
  char *p = engine.nextConversionBuffer.keyCodeBuffer.ToString();
  if (!Str::Eq(p, expected)) {
    printf("Expected: %s\nActual: %s\n", expected, p);
    assert(Str::Eq(p, expected));
  }
  free(p);
}

void StenoEngineTester::TestSymbols(StenoEngine &engine) {
  // spellchecker: disable
  engine.ProcessStroke(StenoStroke("SKWHEUFPL"));
  engine.ProcessStroke(StenoStroke("SKWHEUFPL"));
  engine.ProcessStroke(StenoStroke("SKWHEFG"));
  // spellchecker: enable
  assert(engine.nextConversionBuffer.keyCodeBuffer.count == 4);
  assert(engine.nextConversionBuffer.keyCodeBuffer.buffer[0] ==
         StenoKeyCode('{', StenoCaseMode::NORMAL));
  assert(engine.nextConversionBuffer.keyCodeBuffer.buffer[1] ==
         StenoKeyCode('{', StenoCaseMode::NORMAL));
  assert(engine.nextConversionBuffer.keyCodeBuffer.buffer[2] ==
         StenoKeyCode::CreateRawKeyCodePress(KeyCode::BACKSPACE));
  assert(engine.nextConversionBuffer.keyCodeBuffer.buffer[3] ==
         StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::BACKSPACE));
}

TEST_BEGIN("Engine: Test symbols") {
  StenoDictionaryList dictionary(DICTIONARIES, 2);

  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(dictionary, orthography);
  StenoEngineTester::TestSymbols(engine);
}
TEST_END

void StenoEngineTester::TestAddTranslation(StenoEngine &engine) {
  // spellchecker: disable
  engine.ProcessStroke(StenoStroke("KAT"));
  VerifyTextBuffer(engine, "KAT");
  engine.ProcessUndo();

  engine.ProcessStroke(StenoStroke("TKUPT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: ");

  engine.ProcessStroke(StenoStroke("R-R"));
  engine.ProcessStroke(StenoStroke("KAT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: KAT");

  engine.ProcessStroke(StenoStroke("R-R"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: KAT; "
                           "Translation: ");

  engine.ProcessStroke(StenoStroke("TEFT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: KAT; "
                           "Translation: test");

  engine.ProcessStroke(StenoStroke("R-R"));
  VerifyTextBuffer(engine, "");

  engine.ProcessStroke(StenoStroke("KAT"));
  VerifyTextBuffer(engine, "test");
  // spellchecker: enable
}

void StenoEngineTester::TestScancodeAddTranslation(StenoEngine &engine) {
  // spellchecker: disable
  engine.ProcessStroke(StenoStroke("KAT"));
  VerifyTextBuffer(engine, "KAT");
  engine.ProcessUndo();

  engine.ProcessStroke(StenoStroke("TKUPT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: ");

  engine.ProcessStroke(StenoStroke("R-R"));
  engine.ProcessStroke(StenoStroke("KAT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: KAT");

  engine.ProcessStroke(StenoStroke("R-R"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: KAT; "
                           "Translation: ");

  engine.ProcessScanCode(KeyCode::D, ScanCodeAction::TAP);
  engine.ProcessScanCode(KeyCode::O, ScanCodeAction::TAP);
  engine.ProcessScanCode(KeyCode::G, ScanCodeAction::TAP);
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Strokes: KAT; "
                           "Translation: dog");

  engine.ProcessScanCode(KeyCode::ENTER, ScanCodeAction::TAP);
  VerifyTextBuffer(engine, "");

  engine.ProcessStroke(StenoStroke("KAT"));
  VerifyTextBuffer(engine, "dog");
  // spellchecker: enable
}

TEST_BEGIN("Engine: Random spam") {
  static StenoDictionary *DICTIONARIES[] = {
      &StenoJeffShowStrokeDictionary::instance,
      &StenoJeffPhrasingDictionary::instance,
      &StenoJeffNumbersDictionary::instance,
      &StenoEmilySymbolsDictionary::instance,
      &mainDictionary,
      &testDictionary,
  };

  StenoDictionaryList dictionaryList(
      DICTIONARIES, sizeof(DICTIONARIES) / sizeof(*DICTIONARIES)); // NOLINT

  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  // StenoCompiledOrthography orthography(testOrthography);
  StenoEngine engine(dictionaryList, orthography);
  engine.EnablePaperTape();
  engine.EnableTextLog();
  engine.EnableSuggestions();

  Key::DisableHistory();

  srand(0x1234);
  for (size_t i = 0; i < 10000; ++i) {
    const StenoStroke stroke(rand() & StrokeMask::ALL);
    engine.ProcessStroke(stroke);
  }

  Key::EnableHistory();
}
TEST_END

TEST_BEGIN("Engine: Add Translation Test") {
  const StenoEngineTester tester;
  uint8_t *buffer = new uint8_t[512 * 1024];
  const StenoUserDictionaryData layout(buffer, 512 * 1024);
  StenoUserDictionary *userDictionary = new StenoUserDictionary(layout);

  static StenoDictionary *dictionaries[] = {
      userDictionary,
      &testDictionary,
  };

  StenoDictionaryList dictionaryList(
      dictionaries, sizeof(dictionaries) / sizeof(*dictionaries)); // NOLINT
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(dictionaryList, orthography, userDictionary);
  tester.TestAddTranslation(engine);

  delete userDictionary;
  delete[] buffer;
}
TEST_END

TEST_BEGIN("Engine: Scancode Add Translation Test") {
  const StenoEngineTester tester;
  uint8_t *buffer = new uint8_t[512 * 1024];
  const StenoUserDictionaryData layout(buffer, 512 * 1024);
  StenoUserDictionary *userDictionary = new StenoUserDictionary(layout);

  static StenoDictionary *dictionaries[] = {
      userDictionary,
      &testDictionary,
      &StenoUnicodeDictionary::instance,
  };

  StenoDictionaryList dictionaryList(
      dictionaries, sizeof(dictionaries) / sizeof(*dictionaries)); // NOLINT
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(dictionaryList, orthography, userDictionary);
  tester.TestScancodeAddTranslation(engine);

  delete userDictionary;
  delete[] buffer;
}
TEST_END

//---------------------------------------------------------------------------

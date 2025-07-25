//---------------------------------------------------------------------------

#include "engine.h"

#include JAVELIN_BOARD_CONFIG

#include "console.h"
#include "dictionary/user_dictionary.h"
#include "flash.h"
#include "hal/external_flash.h"
#include "host_layout.h"
#include "orthography.h"
#include "processor/paper_tape.h"
#include "segment.h"
#include "steno_key_code_buffer.h"
#include "str.h"
#include "stroke.h"
#include "word_list.h"

//---------------------------------------------------------------------------

#if JAVELIN_USE_EMBEDDED_STENO
JavelinStaticAllocate<StenoEngine> StenoEngine::container;
#endif

//---------------------------------------------------------------------------

void StenoEngine::TemplateValue::Set(char *newValue) {
  free(value);
  value = newValue;
}

//---------------------------------------------------------------------------

StenoEngine::StenoEngine(StenoDictionary &dictionary,
                         const StenoCompiledOrthography &orthography,
                         StenoStroke undoStroke,
                         StenoUserDictionary *userDictionary)
    : undoStroke(undoStroke), activeDictionary(&dictionary),
      storedDictionaries(&dictionary), orthography(orthography),
      userDictionary(userDictionary) {
  previousConversionBuffer.Prepare(&this->orthography, &GetDictionary());
  nextConversionBuffer.Prepare(&this->orthography, &GetDictionary());
  ResetState();
}

//---------------------------------------------------------------------------

void StenoEngine::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }

  Process(value.ToStroke());
}

void StenoEngine::Process(StenoStroke stroke) {
  ++strokeCount;
  if (stroke == undoStroke) {
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
  switch (mode) {
  case StenoEngineMode::NORMAL:
    return false;

  case StenoEngineMode::ADD_TRANSLATION: {
    const ExternalFlashSentry externalFlashSentry;
    return HandleAddTranslationModeScanCode(scanCodeAndModifiers, action);
  }

  case StenoEngineMode::CONSOLE: {
    const ExternalFlashSentry externalFlashSentry;
    return HandleConsoleModeScanCode(scanCodeAndModifiers, action);
  }
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
  Console::Printf("    Host layout: %s\n",
                  HostLayouts::GetActiveLayout().GetName());
  Console::Printf("    Space position: %s\n",
                  placeSpaceAfter ? "after" : "before");

  orthography.PrintInfo();

  Console::Printf("    Dictionaries\n");
  GetDictionary().PrintInfo(4);
}

void StenoEngine::PrintDictionary(const char *name) const {
  const ExternalFlashSentry externalFlashSentry;

  Console::Printf("{");
  ConsolePrintDictionaryContext context(name);
  GetDictionary().PrintDictionary(context);
  Console::Printf("\n}\n\n");
}

void StenoEngine::ListDictionaries() const {
  const ExternalFlashSentry externalFlashSentry;
  GetDictionary().ListDictionaries();
}

bool StenoEngine::EnableDictionary(const char *name) {
  const ExternalFlashSentry externalFlashSentry;
  return GetDictionary().EnableDictionary(name);
}

bool StenoEngine::DisableDictionary(const char *name) {
  const ExternalFlashSentry externalFlashSentry;
  return GetDictionary().DisableDictionary(name);
}

bool StenoEngine::ToggleDictionary(const char *name) {
  const ExternalFlashSentry externalFlashSentry;
  return GetDictionary().ToggleDictionary(name);
}

void StenoEngine::ReverseLookup(StenoReverseDictionaryLookup &lookup) const {
  const ExternalFlashSentry externalFlashSentry;

  GetDictionary().ReverseLookup(lookup);
  if (lookup.results.IsEmpty()) {
    return;
  }

  lookup.SortResults();
}

void StenoEngine::SendText(const uint8_t *p) {
  const ExternalFlashSentry externalFlashSentry;

  const char *ccp = (const char *)p;

  nextConversionBuffer.keyCodeBuffer.Reset();
  nextConversionBuffer.keyCodeBuffer.AppendTextNoCaseModeOverride(
      ccp, Str::Length(ccp), StenoCaseMode::NORMAL);

  emitter.Emit(nextConversionBuffer.keyCodeBuffer);
}

void StenoEngine::SetTemplateValue(size_t index, char *data) {
  if (index >= TEMPLATE_VALUE_COUNT) {
    free(data);
    return;
  }
  templateValues[index].Set(data);
  if (Console::IsEventEnabled(ConsoleEvent::TEMPLATE_VALUE)) {
    Console::Printf("EV {\"e\":\"v\",\"i\":%zu,\"v\":\"%J\"}\n\n", index, data);
  }
}

char *StenoEngine::SwapTemplateValue(size_t index, char *data) {
  if (index >= TEMPLATE_VALUE_COUNT) {
    return nullptr;
  }
  char *oldValue = templateValues[index].value;
  templateValues[index].value = data;
  return oldValue;
}

char *StenoEngine::ConvertText(StenoSegmentList &segments,
                               size_t startingOffset) {
  StenoKeyCodeBuffer &keyCodeBuffer = nextConversionBuffer.keyCodeBuffer;
  keyCodeBuffer.Reset();
  if (startingOffset < segments.GetCount()) {
    ConvertText(keyCodeBuffer, segments, startingOffset, false);
  }
  return keyCodeBuffer.ToString();
}

[[gnu::weak]] void StenoEngine::Pump() {}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#if RUN_TESTS

#include "key.h"
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
#include "dictionary/user_dictionary.h"

#define DO_PROFILE_TEST 0

extern StenoOrthography testOrthography;

#if DO_PROFILE_TEST
extern StenoCompactMapDictionaryDefinition testDictionaryDefinition;
static StenoCompactMapDictionary mainDictionary(testDictionaryDefinition);
#else
static StenoCompactMapDictionary mainDictionary(TestDictionary::definition);
#endif

static StenoCompactMapDictionary testDictionary(TestDictionary::definition);

StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::specifySpacesInstance,
    &mainDictionary,
};

class StenoEngineTester {
public:
  static void TestSymbols(StenoEngine &engine);
  static void TestTransform(StenoEngine &engine);
  static void TestSetTransform(StenoEngine &engine);
  static void TestEngine(StenoEngine &engine);
  static void TestAddTranslation(StenoEngine &engine);
  static void TestScancodeAddTranslation(StenoEngine &engine);
  static void TestRetroInsertSpace(StenoEngine &engine);
  static void TestRetroInsertSpaceAutoSuffix(StenoEngine &engine);
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
  assert(engine.nextConversionBuffer.keyCodeBuffer.GetCount() == 4);
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

void StenoEngineTester::TestTransform(StenoEngine &engine) {
  engine.SetTemplateValue(0, Str::Dup("c"));
  engine.ProcessStroke(StenoStroke("S"));
  engine.ProcessStroke(StenoStroke("T"));
  engine.ProcessStroke(StenoStroke("K"));
  char *text = engine.nextConversionBuffer.keyCodeBuffer.ToString();
  assert(Str::Eq(engine.GetTemplateValue(0), "acb"));
  assert(Str::Eq(text, "xacb"));
  free(text);
}

TEST_BEGIN("Engine: Test transform") {
  uint8_t *buffer = new uint8_t[512 * 1024];
  memset(buffer, 0, 512 * 1024);
  const StenoUserDictionaryData layout(buffer, 512 * 1024);
  StenoUserDictionary userDictionary(layout);

  const StenoStroke stroke("S");
  userDictionary.Add(&stroke, 1, "=transform:a%0b");
  const StenoStroke setValueStroke("T");
  userDictionary.Add(&setValueStroke, 1, "=set_value:0:1");
  const StenoStroke useValueStroke("K");
  userDictionary.Add(&useValueStroke, 1, "=transform:x%0");

  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(userDictionary, orthography);
  StenoEngineTester::TestTransform(engine);

  delete[] buffer;
}
TEST_END

void StenoEngineTester::TestSetTransform(StenoEngine &engine) {
  engine.ProcessStroke(StenoStroke("S"));
  engine.ProcessStroke(StenoStroke("K"));
  char *text = engine.nextConversionBuffer.keyCodeBuffer.ToString();
  assert(Str::Eq(engine.GetTemplateValue(0), "cat"));
  assert(Str::Eq(text, "xcat"));

  engine.ProcessStroke(StenoStroke("T"));
  engine.ProcessStroke(StenoStroke("K"));
  text = engine.nextConversionBuffer.keyCodeBuffer.ToString();
  assert(Str::Eq(engine.GetTemplateValue(0), "dog"));
  assert(Str::Eq(text, "xcat xdog"));
  free(text);
}

TEST_BEGIN("Engine: Test set_value and transform") {
  uint8_t *buffer = new uint8_t[512 * 1024];
  memset(buffer, 0, 512 * 1024);
  const StenoUserDictionaryData layout(buffer, 512 * 1024);
  StenoUserDictionary userDictionary(layout);

  const StenoStroke stroke("S");
  userDictionary.Add(&stroke, 1, "cat");
  const StenoStroke setValueStroke("T");
  userDictionary.Add(&setValueStroke, 1, "dog");
  const StenoStroke useValueStroke("K");
  userDictionary.Add(&useValueStroke, 1, "=set_value:0:1 =transform:x%0");

  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(userDictionary, orthography);
  StenoEngineTester::TestSetTransform(engine);

  delete[] buffer;
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
      &StenoEmilySymbolsDictionary::specifySpacesInstance,
      &mainDictionary,
      &testDictionary,
  };

  StenoDictionaryList dictionaryList(
      DICTIONARIES, sizeof(DICTIONARIES) / sizeof(*DICTIONARIES)); // NOLINT

  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  // StenoCompiledOrthography orthography(testOrthography);
  StenoEngine engine(dictionaryList, orthography);
  Console::EnableEvent(ConsoleEvent::PAPER_TAPE);
  Console::EnableEvent(ConsoleEvent::TEXT);
  Console::EnableEvent(ConsoleEvent::SUGGESTION);

  Key::DisableHistory();

  srand(0x1234);
#if DO_PROFILE_TEST
  const size_t iterationCount = 1'000'000;
#else
  const size_t iterationCount = 1000;
#endif
  for (size_t i = 0; i < iterationCount; ++i) {
    const StenoStroke stroke(rand() & StrokeMask::ALL);
    engine.ProcessStroke(stroke);
  }

  Key::EnableHistory();
  Console::DisableEvent(ConsoleEvent::PAPER_TAPE);
  Console::DisableEvent(ConsoleEvent::TEXT);
  Console::DisableEvent(ConsoleEvent::SUGGESTION);
}
TEST_END

TEST_BEGIN("Engine: Add Translation Test") {
  const StenoEngineTester tester;
  uint8_t *buffer = new uint8_t[512 * 1024];
  memset(buffer, 0, 512 * 1024);
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
  StenoEngine engine(dictionaryList, orthography, StenoStroke(StrokeMask::STAR),
                     userDictionary);
  tester.TestAddTranslation(engine);

  delete userDictionary;
  delete[] buffer;
}
TEST_END

TEST_BEGIN("Engine: Scancode Add Translation Test") {
  const StenoEngineTester tester;
  uint8_t *buffer = new uint8_t[512 * 1024];
  memset(buffer, 0, 512 * 1024);
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
  StenoEngine engine(dictionaryList, orthography, StenoStroke(StrokeMask::STAR),
                     userDictionary);
  tester.TestScancodeAddTranslation(engine);

  delete userDictionary;
  delete[] buffer;
}
TEST_END

void StenoEngineTester::TestRetroInsertSpace(StenoEngine &engine) {
  engine.ProcessStroke(StenoStroke("TEFT"));
  VerifyTextBuffer(engine, "test");

  engine.ProcessStroke(StenoStroke("SKWHU"));
  VerifyTextBuffer(engine, "test");

  engine.ProcessStroke(StenoStroke("TEFT"));
  VerifyTextBuffer(engine, "test test");

  engine.ProcessStroke(StenoStroke("-D"));
  VerifyTextBuffer(engine, " tested");

  engine.ProcessStroke(StenoStroke("SKWHU"));
  VerifyTextBuffer(engine, " test -D");

  engine.ProcessStroke(StenoStroke("TEFT"));
  VerifyTextBuffer(engine, "test test -D test");
}

TEST_BEGIN("Engine: Verify =retro_insert_space") {
  StenoDictionaryList dictionaryList(
      DICTIONARIES, sizeof(DICTIONARIES) / sizeof(*DICTIONARIES)); // NOLINT
  const StenoCompiledOrthography orthography(
      StenoOrthography::emptyOrthography);
  StenoEngine engine(dictionaryList, orthography);

  const StenoEngineTester tester;
  tester.TestRetroInsertSpace(engine);
}
TEST_END

void StenoEngineTester::TestRetroInsertSpaceAutoSuffix(StenoEngine &engine) {
  engine.ProcessStroke(StenoStroke("TEFTD"));
  VerifyTextBuffer(engine, "tested");

  engine.ProcessStroke(StenoStroke("SKWHU"));
  VerifyTextBuffer(engine, "tested");

  engine.ProcessStroke(StenoStroke("TEFTD"));
  VerifyTextBuffer(engine, " tested");

  engine.ProcessStroke(StenoStroke("TEFTD"));
  VerifyTextBuffer(engine, " tested tested");

  engine.ProcessStroke(StenoStroke("TEFTD"));
  VerifyTextBuffer(engine, " tested");

  engine.ProcessStroke(StenoStroke("TEFTD"));
  VerifyTextBuffer(engine, " tested");
}

TEST_BEGIN("Engine: Verify =retro_insert_space with auto-suffix") {
  StenoDictionaryList dictionaryList(
      DICTIONARIES, sizeof(DICTIONARIES) / sizeof(*DICTIONARIES)); // NOLINT
  const StenoCompiledOrthography orthography(testOrthography);
  StenoEngine engine(dictionaryList, orthography);

  const StenoEngineTester tester;
  tester.TestRetroInsertSpaceAutoSuffix(engine);
}
TEST_END

//---------------------------------------------------------------------------
#endif // RUN_TESTS
//---------------------------------------------------------------------------

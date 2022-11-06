//---------------------------------------------------------------------------

#include "engine.h"

#include "chord.h"
#include "clock.h"
#include "config_block.h"
#include "console.h"
#include "dictionary/user_dictionary.h"
#include "flash.h"
#include "key_code.h"
#include "list.h"
#include "orthography.h"
#include "pattern.h"
#include "segment.h"
#include "steno_key_code_buffer.h"
#include "str.h"
#include "word_list.h"
#include <string.h>

//---------------------------------------------------------------------------

constexpr StenoChord StenoEngine::UNDO_CHORD(ChordMask::STAR);

//---------------------------------------------------------------------------

StenoEngine::StenoEngine(const StenoDictionary &dictionary,
                         const StenoOrthography &orthography,
                         StenoUserDictionary *userDictionary)
    : dictionary(dictionary), orthography(orthography),
      userDictionary(userDictionary) {}

//---------------------------------------------------------------------------

void StenoEngine::Process(StenoKeyState value, StenoAction action) {
  if (action != StenoAction::TRIGGER)
    return;

  ++strokeCount;
  StenoChord chord = value.ToChord();
  if (chord == UNDO_CHORD) {
    ProcessUndo();
  } else {
    ProcessChord(chord);
  }
}

void StenoEngine::ProcessChord(StenoChord chord) {
  switch (mode) {
  case StenoEngineMode::NORMAL:
    ProcessNormalModeChord(chord);
    return;

  case StenoEngineMode::ADD_TRANSLATION:
    ProcessAddTranslationModeChord(chord);
    return;
  }
}

void StenoEngine::ProcessUndo() {
  switch (mode) {
  case StenoEngineMode::NORMAL:
    ProcessNormalModeUndo();
    return;

  case StenoEngineMode::ADD_TRANSLATION:
    ProcessAddTranslationModeUndo();
    return;
  }
}

//---------------------------------------------------------------------------

void StenoEngine::PrintInfo() const {
  Console::Printf("  Javelin Steno Engine\n");
  Console::Printf("    Strokes: %u\n", strokeCount);
  Console::Printf("    Unicode mode: %s\n", emitter.GetUnicodeModeName());
  Console::Printf("    Keyboard layout: %s\n", Key::GetKeyboardLayoutName());

  orthography.PrintInfo();

  Console::Printf("    Dictionaries\n");
  dictionary.PrintInfo();
}

void StenoEngine::PrintDictionary() const {
  Console::Write("{", 1);
  dictionary.PrintDictionary(false);
  Console::Write("\n}\n\n", 4);
}

//---------------------------------------------------------------------------

#include "key_code.h"
#include "unit_test.h"
#include <stdio.h>

#include "dictionary/dictionary_list.h"
#include "dictionary/emily_symbols_dictionary.h"
#include "dictionary/jeff_numbers_dictionary.h"
#include "dictionary/jeff_phrasing_dictionary.h"
#include "dictionary/jeff_show_stroke_dictionary.h"
#include "dictionary/main_dictionary.h"
#include "dictionary/map_dictionary.h"

constexpr StenoMapDictionary mainDictionary(MainDictionary::definition);

const StenoDictionary *const DICTIONARIES[] = {
    &StenoEmilySymbolsDictionary::instance,
    &mainDictionary,
};

constexpr StenoDictionaryList dictionary(DICTIONARIES, 2);

class StenoEngineTester {
public:
  static void TestSymbols(StenoEngine &engine);
  static void TestEngine(StenoEngine &engine);
  static void TestAddTranslation(StenoEngine &engine);
  static void VerifyTextBuffer(StenoEngine &engine, const char *expected);
};

void StenoEngineTester::VerifyTextBuffer(StenoEngine &engine,
                                         const char *expected) {
  char *p = engine.nextKeyCodeBuffer.ToString();
  if (!Str::Eq(p, expected)) {
    printf("Expected: %s\nActual: %s\n", expected, p);
    assert(Str::Eq(p, expected));
  }
  free(p);
}

void StenoEngineTester::TestSymbols(StenoEngine &engine) {
  // spellchecker: disable
  engine.ProcessChord(StenoChord("SKWHEUFPL"));
  engine.ProcessChord(StenoChord("SKWHEUFPL"));
  engine.ProcessChord(StenoChord("SKWHEFG"));
  // spellchecker: enable
  assert(engine.nextKeyCodeBuffer.count == 4);
  assert(engine.nextKeyCodeBuffer.buffer[0] ==
         StenoKeyCode('{', StenoCaseMode::NORMAL));
  assert(engine.nextKeyCodeBuffer.buffer[1] ==
         StenoKeyCode('{', StenoCaseMode::NORMAL));
  assert(engine.nextKeyCodeBuffer.buffer[2] ==
         StenoKeyCode::CreateRawKeyCodePress(KeyCode::BACKSPACE));
  assert(engine.nextKeyCodeBuffer.buffer[3] ==
         StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::BACKSPACE));
}

TEST_BEGIN("Engine: Test symbols") {
  StenoEngine engine(dictionary, StenoOrthography::emptyOrthography);
  StenoEngineTester::TestSymbols(engine);
}
TEST_END

void StenoEngineTester::TestAddTranslation(StenoEngine &engine) {
  // spellchecker: disable
  engine.ProcessChord(StenoChord("KAT"));
  VerifyTextBuffer(engine, "KAT");
  engine.ProcessUndo();

  engine.ProcessChord(StenoChord("TKUPT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Chords: ");

  engine.ProcessChord(StenoChord("R-R"));
  engine.ProcessChord(StenoChord("KAT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Chords: KAT");

  engine.ProcessChord(StenoChord("R-R"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Chords: KAT; "
                           "Translation: ");

  engine.ProcessChord(StenoChord("TEFT"));
  VerifyTextBuffer(engine, " >>> Add/Delete Translation - Chords: KAT; "
                           "Translation: test");

  engine.ProcessChord(StenoChord("R-R"));
  VerifyTextBuffer(engine, "");

  engine.ProcessChord(StenoChord("KAT"));
  VerifyTextBuffer(engine, "test");
  // spellchecker: enable
}

TEST_BEGIN("Engine: Random spam") {
  static const StenoDictionary *DICTIONARIES[] = {
      &StenoJeffShowStrokeDictionary::instance,
      &StenoJeffPhrasingDictionary::instance,
      &StenoJeffNumbersDictionary::instance,
      &StenoEmilySymbolsDictionary::instance,
      &mainDictionary,
  };

  static constexpr StenoDictionaryList dictionaryList(
      DICTIONARIES, sizeof(DICTIONARIES) / sizeof(*DICTIONARIES)); // NOLINT

  StenoEngine engine(dictionaryList, StenoOrthography::emptyOrthography);

  Key::DisableHistory();

  srand(0x1234);
  for (size_t i = 0; i < 1000; ++i) {
    StenoChord chord(rand() & ChordMask::ALL);
    engine.ProcessChord(chord);
  }

  Key::EnableHistory();
}
TEST_END

TEST_BEGIN("Engine: Add Translation Test") {
  StenoEngineTester tester;
  uint8_t *buffer = new uint8_t[512 * 1024];
  StenoUserDictionaryData layout(buffer, 512 * 1024);
  StenoUserDictionary *userDictionary = new StenoUserDictionary(layout);

  static const StenoDictionary *dictionaries[] = {
      userDictionary,
      &mainDictionary,
  };

  static constexpr StenoDictionaryList dictionaryList(
      dictionaries, sizeof(dictionaries) / sizeof(*dictionaries)); // NOLINT
  StenoEngine engine(dictionaryList, StenoOrthography::emptyOrthography,
                     userDictionary);
  tester.TestAddTranslation(engine);

  delete userDictionary;
  delete[] buffer;
}
TEST_END

//---------------------------------------------------------------------------

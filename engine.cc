//---------------------------------------------------------------------------

#include "engine.h"

#include "chord.h"
#include "clock.h"
#include "config_block.h"
#include "console.h"
#include "dictionary/user_dictionary.h"
#include "flash.h"
#include "list.h"
#include "orthography.h"
#include "pattern.h"
#include "segment.h"
#include "steno_key_code_buffer.h"
#include "string_util.h"
#include "word_list.h"
#include <string.h>

//---------------------------------------------------------------------------

constexpr StenoChord StenoEngine::UNDO_CHORD(ChordMask::STAR);
StenoEngineMode StenoEngine::mode;

//---------------------------------------------------------------------------

struct StenoEngine::SuffixEntry {
  SuffixEntry();
  SuffixEntry(char *text, int score) : text(text), score(score) {}

  char *text;
  int score;
};

//---------------------------------------------------------------------------

StenoEngine::StenoEngine(const StenoDictionary &dictionary,
                         const StenoOrthography &orthography,
                         StenoUserDictionary *userDictionary)
    : dictionary(dictionary), orthography(orthography),
      patterns(CreatePatterns(orthography)), userDictionary(userDictionary) {}

const Pattern *
StenoEngine::CreatePatterns(const StenoOrthography &orthography) {
  Pattern *patterns =
      (Pattern *)malloc(sizeof(Pattern) * orthography.ruleCount);
  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    patterns[i] = Pattern::Compile(orthography.rules[i].testPattern);
  }
  return patterns;
}

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

char *StenoEngine::AddSuffix(const char *word, const char *suffix) const {
  List<SuffixEntry> candidates;

  for (size_t i = 0; i < orthography.aliasCount; ++i) {
    if (streq(word, orthography.aliases[i].text)) {
      AddCandidates(candidates, word, orthography.aliases[i].alias);
    }
  }

  char *simple = rasprintf("%s%s", word, suffix);
  int score = WordList::GetWordRank(simple);
  if (score >= 0) {
    candidates.Add(SuffixEntry(simple, score));
  } else {
    free(simple);
  }

  AddCandidates(candidates, word, suffix);

  if (candidates.IsNotEmpty()) {
    candidates.Sort([](const void *pa, const void *pb) -> int {
      const SuffixEntry *a = (const SuffixEntry *)pa;
      const SuffixEntry *b = (const SuffixEntry *)pb;
      if (a->score != b->score) {
        return a->score - b->score;
      }
      return (int)(a - b);
    });
    for (size_t i = 1; i < candidates.GetCount(); ++i) {
      free(candidates[i].text);
    }
    return candidates[0].text;
  }
  for (size_t i = 0; i < candidates.GetCount(); ++i) {
    free(candidates[i].text);
  }

  char *text = rasprintf("%s ^%s", word, suffix);
  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    const PatternMatch match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(orthography.rules[i].replacement);
    free(text);
    return candidate;
  }

  free(text);
  return rasprintf("%s%s", word, suffix);
}

void StenoEngine::AddCandidates(List<SuffixEntry> &candidates, const char *word,
                                const char *suffix) const {
  char *text = rasprintf("%s ^%s", word, suffix);

  for (size_t i = 0; i < orthography.ruleCount; ++i) {
    const PatternMatch match = patterns[i].Match(text);
    if (!match.match) {
      continue;
    }

    char *candidate = match.Replace(orthography.rules[i].replacement);
    int score = WordList::GetWordRank(candidate);
    if (score < 0) {
      free(candidate);
      continue;
    }
    candidates.Add(SuffixEntry(candidate, score));
  }
  free(text);
}

//---------------------------------------------------------------------------

void StenoEngine::PrintInfo(const StenoConfigBlock *configBlock) const {
  uint32_t uptime = Clock::GetCurrentTime();
  uint32_t microseconds = uptime % 1000;
  uint32_t totalSeconds = uptime / 1000;
  uint32_t seconds = totalSeconds % 60;
  uint32_t totalMinutes = totalSeconds / 60;
  uint32_t minutes = totalMinutes % 60;
  uint32_t totalHours = totalMinutes / 60;
  uint32_t hours = totalHours % 24;
  uint32_t days = totalHours / 24;

  Console::Printf("Uptime: %ud %uh %um %0u.%03us\n", days, hours, minutes,
                  seconds, microseconds);
  Console::Printf("Strokes: %u\n", strokeCount);

  Console::Printf("Options\n");
  Console::Printf("  Unicode mode: %s\n", emitter.GetUnicodeModeName());
  if (configBlock) {
    Console::Printf("  First Up: %s\n", configBlock->useFirstUp ? "on" : "off");
    Console::Printf("  Repeat: %s\n", configBlock->useRepeat ? "on" : "off");
  }

  Flash::PrintInfo();

  Console::Printf("Dictionaries\n");
  if (configBlock && configBlock->useJeffShowStroke) {
    Console::Printf("  Jeff's Modifiers\n");
  }
  dictionary.PrintInfo();
  Console::Printf("Orthography\n");
  Console::Printf("  Rules: %zu\n", orthography.ruleCount);
  Console::Printf("  Aliases: %zu\n", orthography.aliasCount);

  Console::Write("\n", 1);
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
  if (!streq(p, expected)) {
    printf("Expected: %s\nActual: %s\n", expected, p);
    assert(streq(p, expected));
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

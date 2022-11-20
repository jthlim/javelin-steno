//---------------------------------------------------------------------------

#include "jeff_show_stroke_dictionary.h"
#include "../chord.h"
#include "../console.h"
#include <string.h>

//---------------------------------------------------------------------------

constexpr StenoChord StenoJeffShowStrokeDictionary::trigger(ChordMask::SL |
                                                            ChordMask::TL |
                                                            ChordMask::RL |
                                                            ChordMask::STAR);

const StenoJeffShowStrokeDictionary StenoJeffShowStrokeDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult StenoJeffShowStrokeDictionary::Lookup(
    const StenoDictionaryLookup &lookup) const {
  const StenoChord *chords = lookup.chords;
  if (chords[0] != trigger) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t length = lookup.length;
  if (length == 1) {
    return StenoDictionaryLookupResult::CreateStaticString("`");
  }
  for (size_t i = 1; i < length - 1; ++i) {
    if (chords[i] == trigger) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }

  const size_t maximumChordText = ChordBitIndex::COUNT;
  size_t maximumStringLength =
      (maximumChordText + 1) * (GetMaximumMatchLength() - 1) + 2;

  bool closed = (chords[length - 1] == trigger);

  char *base = (char *)malloc(maximumStringLength);
  char *p = base;
  *p++ = '`';

  size_t end = closed ? length - 1 : length;
  p = StenoChord::ToString(chords + 1, end - 1, p);
  if (closed) {
    p[0] = '`';
    p[1] = '\0';
  }

  return StenoDictionaryLookupResult::CreateDynamicString(base);
}

const StenoDictionary *StenoJeffShowStrokeDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  const StenoChord *chords = lookup.chords;
  if (chords[0] != trigger) {
    return nullptr;
  }

  size_t length = lookup.length;
  if (length == 1) {
    return this;
  }
  for (size_t i = 1; i < length - 1; ++i) {
    if (chords[i] == trigger) {
      return nullptr;
    }
  }

  return this;
}

const char *StenoJeffShowStrokeDictionary::GetName() const {
  return "jeff_show_stroke";
}

//---------------------------------------------------------------------------

#include "../unit_test.h"

#include <assert.h>

// cSpell:ignore TKOG
TEST_BEGIN("JeffShowStroke: Non-quoted test") {
  const StenoChord chords[1] = {
      StenoChord("KAT"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(chords, 1);
  assert(!lookup.IsValid());
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: First quote test") {
  const StenoChord chords[1] = {
      StenoChord("STR*"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(chords, 1);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "`") == 0);
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: In-progress quote") {
  const StenoChord chords[2] = {
      StenoChord("STR*"),
      StenoChord("KAT"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(chords, 2);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "`KAT") == 0);
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: In-progress, multiple stroke test") {
  const StenoChord chords[3] = {
      StenoChord("STR*"),
      StenoChord("KAT"),
      StenoChord("TKOG"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(chords, 3);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "`KAT/TKOG") == 0);
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: Completed quote test") {
  const StenoChord chords[4] = {
      StenoChord("STR*"),
      StenoChord("KAT"),
      StenoChord("TKOG"),
      StenoChord("STR*"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(chords, 4);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "`KAT/TKOG`") == 0);
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: Invalid quote test") {
  const StenoChord chords[5] = {
      StenoChord("STR*"), StenoChord("KAT"), StenoChord("TKOG"),
      StenoChord("STR*"), StenoChord("KAT"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(chords, 5);
  assert(!lookup.IsValid());
  lookup.Destroy();
}
TEST_END

//---------------------------------------------------------------------------

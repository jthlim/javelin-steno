//---------------------------------------------------------------------------

#include "jeff_show_stroke_dictionary.h"
#include "../console.h"
#include "../str.h"
#include "../stroke.h"

//---------------------------------------------------------------------------

constexpr StenoStroke StenoJeffShowStrokeDictionary::trigger(StrokeMask::SL |
                                                             StrokeMask::TL |
                                                             StrokeMask::RL |
                                                             StrokeMask::STAR);

//---------------------------------------------------------------------------

StenoJeffShowStrokeDictionary StenoJeffShowStrokeDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult StenoJeffShowStrokeDictionary::Lookup(
    const StenoDictionaryLookup &lookup) const {
  if (lookup.strokes[0] != trigger) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }
  return LookupInternal(lookup);
}

// Split off to no-inline to help gcc generate better early-out code.
__attribute__((noinline)) StenoDictionaryLookupResult
StenoJeffShowStrokeDictionary::LookupInternal(
    const StenoDictionaryLookup &lookup) const {
  const StenoStroke *strokes = lookup.strokes;
  size_t length = lookup.length;
  if (length == 1) {
    return StenoDictionaryLookupResult::CreateStaticString("`");
  }
  for (size_t i = 1; i < length - 1; ++i) {
    if (strokes[i] == trigger) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }

  bool closed = (strokes[length - 1] == trigger);
  size_t end = closed ? length - 1 : length;
  char *text = Str::Asprintf(closed ? "`%T`" : "`%T", strokes + 1, end - 1);

  return StenoDictionaryLookupResult::CreateDynamicString(text);
}

const StenoDictionary *StenoJeffShowStrokeDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  const StenoStroke *strokes = lookup.strokes;
  if (strokes[0] != trigger) {
    return nullptr;
  }

  size_t length = lookup.length;
  if (length == 1) {
    return this;
  }
  for (size_t i = 1; i < length - 1; ++i) {
    if (strokes[i] == trigger) {
      return nullptr;
    }
  }

  return this;
}

const char *StenoJeffShowStrokeDictionary::GetName() const {
  return "jeff-show-stroke";
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

#include <assert.h>

// cSpell:ignore TKOG
TEST_BEGIN("JeffShowStroke: Non-quoted test") {
  const StenoStroke strokes[1] = {
      StenoStroke("KAT"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(strokes, 1);
  assert(!lookup.IsValid());
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: First quote test") {
  const StenoStroke strokes[1] = {
      StenoStroke("STR*"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(strokes, 1);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "`"));
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: In-progress quote") {
  const StenoStroke strokes[2] = {
      StenoStroke("STR*"),
      StenoStroke("KAT"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(strokes, 2);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "`KAT"));
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: In-progress, multiple stroke test") {
  const StenoStroke strokes[3] = {
      StenoStroke("STR*"),
      StenoStroke("KAT"),
      StenoStroke("TKOG"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(strokes, 3);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "`KAT/TKOG"));
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: Completed quote test") {
  const StenoStroke strokes[4] = {
      StenoStroke("STR*"),
      StenoStroke("KAT"),
      StenoStroke("TKOG"),
      StenoStroke("STR*"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(strokes, 4);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "`KAT/TKOG`"));
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("JeffShowStroke: Invalid quote test") {
  const StenoStroke strokes[5] = {
      StenoStroke("STR*"), StenoStroke("KAT"), StenoStroke("TKOG"),
      StenoStroke("STR*"), StenoStroke("KAT"),
  };

  auto lookup = StenoJeffShowStrokeDictionary::instance.Lookup(strokes, 5);
  assert(!lookup.IsValid());
  lookup.Destroy();
}
TEST_END

//---------------------------------------------------------------------------

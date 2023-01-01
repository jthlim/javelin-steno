//---------------------------------------------------------------------------

#include "debug_dictionary.h"
#include "../chord.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

constexpr StenoChord StenoDebugDictionary::trigger(ChordMask::NUM |
                                                   ChordMask::E | ChordMask::U);

const StenoDebugDictionary StenoDebugDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoDebugDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoChord *chords = lookup.chords;
  if (chords[0] != trigger) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  if (response != nullptr) {
    return StenoDictionaryLookupResult::CreateStaticString(response);
  }

  return StenoDictionaryLookupResult::CreateDynamicString(
      Str::Asprintf("Debug text"));
}

const char *StenoDebugDictionary::GetName() const { return "debug"; }

//---------------------------------------------------------------------------

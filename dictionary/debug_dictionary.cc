//---------------------------------------------------------------------------

#include "debug_dictionary.h"
#include "../console.h"
#include "../str.h"
#include "../stroke.h"

//---------------------------------------------------------------------------

constexpr StenoStroke StenoDebugDictionary::trigger(StrokeMask::NUM |
                                                    StrokeMask::E |
                                                    StrokeMask::U);

//---------------------------------------------------------------------------

StenoDebugDictionary StenoDebugDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoDebugDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoStroke *strokes = lookup.strokes;
  if (strokes[0] != trigger) {
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

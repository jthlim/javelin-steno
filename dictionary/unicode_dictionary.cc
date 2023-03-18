//---------------------------------------------------------------------------

#include "unicode_dictionary.h"
#include "../str.h"
#include "../utf8_pointer.h"
#include <assert.h>

//---------------------------------------------------------------------------

StenoUnicodeDictionary StenoUnicodeDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoUnicodeDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke stroke = lookup.strokes[0];
  if ((stroke & StrokeMask::UNICODE).IsEmpty()) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  int unicode = (stroke & ~StrokeMask::UNICODE).GetKeyState();
  if (unicode == '\b') {
    return StenoDictionaryLookupResult::CreateStaticString("{^}\b");
  }

  // Create an infix string so that spaces are inserted properly.
  char buffer[10] = {'{', '^'};
  Utf8Pointer utf8p(buffer + 2);
  utf8p.SetAndAdvance(unicode);
  utf8p.SetAndAdvance('^');
  utf8p.SetAndAdvance('}');
  utf8p.Set(0);

  return StenoDictionaryLookupResult::CreateDynamicString(Str::Dup(buffer));
}

const StenoDictionary *StenoUnicodeDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke stroke = lookup.strokes[0];
  if ((stroke & StrokeMask::UNICODE).IsEmpty()) {
    return nullptr;
  }
  return this;
}

const char *StenoUnicodeDictionary::GetName() const {
  return "#internal#unicode_dictionary";
}

//---------------------------------------------------------------------------

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
  switch (unicode) {
  case '\b':
    return StenoDictionaryLookupResult::CreateStaticString("{^}\b");

  case '\n':
    return StenoDictionaryLookupResult::CreateStaticString("{^~|\n^}");

  default:
    // Create an infix string so that spaces are not inserted.
    char buffer[12] = {'{', '^', '}'};
    Utf8Pointer utf8p(buffer + 3);
    utf8p.SetAndAdvance(unicode);
    utf8p.SetAndAdvance('{');
    utf8p.SetAndAdvance('^');
    utf8p.SetAndAdvance('}');

    return StenoDictionaryLookupResult::CreateDynamicString(
        Str::DupN(buffer, utf8p.GetRawPointer() - buffer));
  }
}

const StenoDictionary *StenoUnicodeDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke stroke = lookup.strokes[0];
  return (stroke & StrokeMask::UNICODE).IsEmpty() ? nullptr : this;
}

const char *StenoUnicodeDictionary::GetName() const {
  return "#internal#unicode_dictionary";
}

//---------------------------------------------------------------------------

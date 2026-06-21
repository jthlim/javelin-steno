//---------------------------------------------------------------------------

#include "unicode_dictionary.h"
#include "../utf8_pointer.h"
#include <assert.h>

//---------------------------------------------------------------------------

StenoUnicodeDictionary StenoUnicodeDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoUnicodeDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke stroke = lookup.strokes[0];
  if ((stroke & StrokeMask::SPECIAL_MASK) != StrokeMask::UNICODE) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  const int unicode = (stroke & ~StrokeMask::SPECIAL_MASK).GetKeyState();
  switch (unicode) {
  case '{':
    return StenoDictionaryLookupResult::CreateStaticString("{^}\\{");

  case '\\':
    return StenoDictionaryLookupResult::CreateStaticString("{^}\\\\");

  case ' ':
    return StenoDictionaryLookupResult::CreateStaticString("{^}\\ ");

  case '\b':
    return StenoDictionaryLookupResult::CreateStaticString("{^}\b");

  case '\n':
    return StenoDictionaryLookupResult::CreateStaticString("{^~|\n^}");

  default:
    // Create an infix string so that spaces are not inserted.
    char buffer[12] = {'{', '^', '}'};
    Utf8Pointer utf8p(buffer + 3);
    utf8p.SetAndAdvance(unicode);
    utf8p.SetAsciiAndAdvance('{');
    utf8p.SetAsciiAndAdvance('^');
    utf8p.SetAsciiAndAdvance('}');

    return StenoDictionaryLookupResult::CreateDupN(
        buffer, utf8p.GetRawPointer() - buffer);
  }
}

const StenoDictionary *StenoUnicodeDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  assert(lookup.length == 1);
  const StenoStroke stroke = lookup.strokes[0];
  return ((stroke & StrokeMask::SPECIAL_MASK) == StrokeMask::UNICODE) ? this
                                                                      : nullptr;
}

const char *StenoUnicodeDictionary::GetName() const {
  return "#internal#unicode_dictionary";
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "corrupted_dictionary.h"

//---------------------------------------------------------------------------

static const char MISMATCHED_MAGIC_ERROR[] =
    "Dictionary Format Error. Please recreate a full firmware";

//---------------------------------------------------------------------------

StenoCorruptedDictionary StenoCorruptedDictionary::instance;

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoCorruptedDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return StenoDictionaryLookupResult::CreateStaticString(
      MISMATCHED_MAGIC_ERROR);
}

//---------------------------------------------------------------------------

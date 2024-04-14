//---------------------------------------------------------------------------

#include "invalid_dictionary.h"

//---------------------------------------------------------------------------

static const char MISMATCHED_MAGIC_ERROR[] =
    "Dictionary Format Error. Please recreate a full firmware";
static const char INCOMPLETE_UPLOAD_ERROR[] =
    "Incomplete dictionary upload detected. Please re-upload firmware.";

//---------------------------------------------------------------------------

StenoInvalidDictionary
    StenoInvalidDictionary::corruptedInstance(MISMATCHED_MAGIC_ERROR);
StenoInvalidDictionary
    StenoInvalidDictionary::incompleteUploadInstance(INCOMPLETE_UPLOAD_ERROR);

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoInvalidDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return StenoDictionaryLookupResult::CreateStaticString(message);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "invalid_dictionary.h"

//---------------------------------------------------------------------------

static constexpr char MISMATCHED_MAGIC_ERROR[] =
    "Dictionary Format Error. Please recreate a full firmware.";
static constexpr char INCOMPLETE_UPLOAD_ERROR[] =
    "Incomplete dictionary upload detected. Please re-upload firmware.";
static constexpr char INVALID_PAIR_VERSION_ERROR[] =
    "Please update the pair firmware.";

//---------------------------------------------------------------------------

StenoInvalidDictionary
    StenoInvalidDictionary::corruptedInstance(MISMATCHED_MAGIC_ERROR);
StenoInvalidDictionary
    StenoInvalidDictionary::incompleteUploadInstance(INCOMPLETE_UPLOAD_ERROR);
StenoInvalidDictionary StenoInvalidDictionary::invalidPairVersionInstance(
    INVALID_PAIR_VERSION_ERROR);

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoInvalidDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return StenoDictionaryLookupResult::CreateStaticString(message);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoInvalidDictionary final : public StenoDictionary {
private:
  static constexpr size_t MAXIMUM_OUTLINE_LENGTH = 8;

public:
  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual bool IsInternal() const { return true; }
  virtual const char *GetName() const { return "#bad"; }

  static StenoInvalidDictionary corruptedInstance;
  static StenoInvalidDictionary incompleteUploadInstance;
  static StenoInvalidDictionary invalidPairVersionInstance;

private:
  StenoInvalidDictionary(const char *message)
      : StenoDictionary(MAXIMUM_OUTLINE_LENGTH), message(message) {}

  const char *const message;
};

//---------------------------------------------------------------------------

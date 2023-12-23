//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Only used for debug test code.
class StenoDebugDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 1;

public:
  StenoDebugDictionary() : StenoDictionary(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const char *GetName() const;

  void SetResponse(const char *p) { response = p; }

  static StenoDebugDictionary instance;

private:
  static const StenoStroke trigger;
  const char *response = nullptr;
};

//---------------------------------------------------------------------------

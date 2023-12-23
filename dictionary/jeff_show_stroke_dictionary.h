//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoJeffShowStrokeDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 6;

public:
  StenoJeffShowStrokeDictionary() : StenoDictionary(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual const char *GetName() const;

  static StenoJeffShowStrokeDictionary instance;

private:
  static const StenoStroke trigger;

  StenoDictionaryLookupResult
  LookupInternal(const StenoDictionaryLookup &lookup) const;
};

//---------------------------------------------------------------------------

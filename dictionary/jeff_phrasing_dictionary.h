//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoJeffPhrasingDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffPhrasingDictionary() {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual unsigned int GetMaximumMatchLength() const { return 1; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const { return false; }

  static const StenoJeffPhrasingDictionary instance;
};

//---------------------------------------------------------------------------

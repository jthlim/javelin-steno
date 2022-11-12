//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoEmilySymbolsDictionary final : public StenoDictionary {
public:
  constexpr StenoEmilySymbolsDictionary() {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual unsigned int GetMaximumMatchLength() const { return 1; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const;

  static const StenoEmilySymbolsDictionary instance;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoEmilySymbolsDictionary final : public StenoDictionary {
public:
  constexpr StenoEmilySymbolsDictionary() : StenoDictionary(1) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual void CacheMaximumOutlineLength() {}
  virtual size_t GetMaximumOutlineLength() const { return 1; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const;

  static const StenoEmilySymbolsDictionary instance;
};

//---------------------------------------------------------------------------

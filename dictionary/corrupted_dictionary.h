//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoCorruptedDictionary final : public StenoDictionary {
public:
  constexpr StenoCorruptedDictionary() : StenoDictionary(1) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual void CacheMaximumOutlineLength() {}
  virtual size_t GetMaximumOutlineLength() const { return 1; }
  virtual const char *GetName() const { return "#bad"; }

  static const StenoCorruptedDictionary instance;
};

//---------------------------------------------------------------------------

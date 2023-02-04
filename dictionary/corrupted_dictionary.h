//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoCorruptedDictionary final : public StenoDictionary {
public:
  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual size_t GetMaximumOutlineLength() const { return 1; }
  virtual const char *GetName() const { return "#bad"; }

  static const StenoCorruptedDictionary instance;
};

//---------------------------------------------------------------------------

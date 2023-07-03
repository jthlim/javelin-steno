//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoCorruptedDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 8;

public:
  constexpr StenoCorruptedDictionary()
      : StenoDictionary(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual void CacheMaximumOutlineLength() {}
  virtual size_t GetMaximumOutlineLength() const {
    return MAXIMUM_OUTLINE_LENGTH;
  }
  virtual const char *GetName() const { return "#bad"; }

  static const StenoCorruptedDictionary instance;
};

//---------------------------------------------------------------------------

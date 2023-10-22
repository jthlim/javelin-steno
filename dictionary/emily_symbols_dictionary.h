//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoEmilySymbolsDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 1;

public:
  constexpr StenoEmilySymbolsDictionary()
      : StenoDictionary(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual void CacheMaximumOutlineLength() {}
  virtual size_t GetMaximumOutlineLength() const {
    return MAXIMUM_OUTLINE_LENGTH;
  }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(const char *name, bool hasData) const;

  static const StenoEmilySymbolsDictionary instance;
};

//---------------------------------------------------------------------------

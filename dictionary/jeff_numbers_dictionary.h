//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Firmware implementation of https://github.com/jthlim/jeff-numbers.
class StenoJeffNumbersDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffNumbersDictionary() : StenoDictionary(1) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual void CacheMaximumOutlineLength() {}
  virtual size_t GetMaximumOutlineLength() const { return 10; }
  virtual const char *GetName() const;

  static const StenoJeffNumbersDictionary instance;

private:
  StenoStroke GetDigits(char *scratch, StenoStroke stroke) const;

  StenoDictionaryLookupResult
  LookupInternal(const StenoDictionaryLookup &lookup) const;
};

//---------------------------------------------------------------------------

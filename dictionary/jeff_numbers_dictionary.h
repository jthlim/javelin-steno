//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Firmware implementation of https://github.com/jthlim/jeff-numbers.
class StenoJeffNumbersDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffNumbersDictionary() {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual unsigned int GetMaximumMatchLength() const { return 10; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const { return false; }

  static const StenoJeffNumbersDictionary instance;

private:
  StenoStroke GetDigits(char *scratch, StenoStroke stroke) const;
};

//---------------------------------------------------------------------------

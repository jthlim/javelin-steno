//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Firmware implementation of https://github.com/jthlim/jeff-numbers.
class StenoJeffNumbersDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 10;

public:
  StenoJeffNumbersDictionary() : StenoDictionary(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const char *GetName() const;

  static StenoJeffNumbersDictionary instance;

private:
  StenoStroke GetDigits(char *scratch, StenoStroke stroke) const;

  StenoDictionaryLookupResult
  LookupInternal(const StenoDictionaryLookup &lookup) const;

  static char *ProcessYear(char *data, int base);
};

//---------------------------------------------------------------------------

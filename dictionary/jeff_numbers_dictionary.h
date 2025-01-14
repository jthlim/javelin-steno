//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Firmware implementation of https://github.com/jthlim/jeff-numbers.
class StenoJeffNumbersDictionary final : public StenoDictionary {
private:
  static constexpr size_t MAXIMUM_OUTLINE_LENGTH = 10;

  using super = StenoDictionary;

public:
  StenoJeffNumbersDictionary() : super(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using super::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;
  using super::GetDictionaryForOutline;

  virtual const char *GetName() const;

  static StenoJeffNumbersDictionary instance;

private:
  StenoStroke GetDigits(char *scratch, StenoStroke stroke) const;

  StenoDictionaryLookupResult
  LookupInternal(const StenoDictionaryLookup &lookup) const;

  static char *ProcessYear(char *data, int base);
};

//---------------------------------------------------------------------------

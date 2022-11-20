//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoJeffShowStrokeDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffShowStrokeDictionary() {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual unsigned int GetMaximumMatchLength() const { return 6; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const { return false; }

  static const StenoJeffShowStrokeDictionary instance;

private:
  static const StenoChord trigger;
};

//---------------------------------------------------------------------------

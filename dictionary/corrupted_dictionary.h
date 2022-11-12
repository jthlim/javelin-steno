//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoCorruptedDictionary final : public StenoDictionary {
public:
  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual unsigned int GetMaximumMatchLength() const { return 1; }
  virtual const char *GetName() const { return "bad"; }
  virtual bool PrintDictionary(bool hasData) const { return hasData; }

  static const StenoCorruptedDictionary instance;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

struct StenoMapDictionaryDefinition;

//---------------------------------------------------------------------------

class StenoMapDictionary final : public StenoDictionary {
public:
  StenoMapDictionary(const StenoMapDictionaryDefinition &definition)
      : definition(definition) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual bool
  ReverseMapDictionaryLookup(StenoReverseMapDictionaryLookup &lookup) const;

  virtual size_t GetMaximumOutlineLength() const;
  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const;

private:
  const StenoMapDictionaryDefinition &definition;
};

//---------------------------------------------------------------------------

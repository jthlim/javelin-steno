//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

struct StenoMapDictionaryDefinition;

//---------------------------------------------------------------------------

class StenoMapDictionary final : public StenoDictionary {
public:
  constexpr StenoMapDictionary(const StenoMapDictionaryDefinition &definition)
      : definition(definition) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual bool ReverseMapDictionaryLookup(StenoReverseDictionaryLookup &result,
                                          const void *data) const;

  virtual unsigned int GetMaximumMatchLength() const;
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const;

private:
  const StenoMapDictionaryDefinition &definition;

  struct DictionaryStats {
    size_t entryCount;
    uint32_t maxTextOffset;
  };
};

//---------------------------------------------------------------------------

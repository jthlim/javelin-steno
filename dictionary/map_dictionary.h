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

  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const final;
  virtual unsigned int GetMaximumMatchLength() const final;
  virtual void PrintInfo() const final;
  virtual bool PrintDictionary(bool hasData) const final;

private:
  const StenoMapDictionaryDefinition &definition;

  struct DictionaryStats {
    size_t entryCount;
    uint32_t maxTextOffset;
  };

  DictionaryStats PrintStrokeInfo() const;
};

//---------------------------------------------------------------------------

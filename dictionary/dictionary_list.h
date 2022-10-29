//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoDictionaryList final : public StenoDictionary {
public:
  constexpr StenoDictionaryList(const StenoDictionary *const *dictionaries,
                                size_t count)
      : dictionaries(dictionaries), count(count) {}

  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const;
  virtual unsigned int GetMaximumMatchLength() const;
  virtual void PrintInfo() const final;
  virtual bool PrintDictionary(bool hasData) const final;

private:
  const StenoDictionary *const *dictionaries;
  size_t count;
};

//---------------------------------------------------------------------------

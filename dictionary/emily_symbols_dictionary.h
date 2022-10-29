//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoEmilySymbolsDictionary final : public StenoDictionary {
public:
  constexpr StenoEmilySymbolsDictionary() {}

  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const final;
  virtual unsigned int GetMaximumMatchLength() const final { return 1; }
  virtual void PrintInfo() const final;
  virtual bool PrintDictionary(bool hasData) const final { return false; }

  static const StenoEmilySymbolsDictionary instance;
};

//---------------------------------------------------------------------------

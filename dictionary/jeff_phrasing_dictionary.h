//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoJeffPhrasingDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffPhrasingDictionary() {}

  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const final;
  virtual unsigned int GetMaximumMatchLength() const final { return 1; }
  virtual void PrintInfo() const final;
  virtual bool PrintDictionary(bool hasData) const final { return false; }

  static const StenoJeffPhrasingDictionary instance;
};

//---------------------------------------------------------------------------

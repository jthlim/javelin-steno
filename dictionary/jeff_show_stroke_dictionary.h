//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoJeffShowStrokeDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffShowStrokeDictionary() {}

  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const final;
  virtual unsigned int GetMaximumMatchLength() const final { return 6; }
  virtual void PrintInfo() const final;
  virtual bool PrintDictionary(bool hasData) const final { return false; }

  static const StenoJeffShowStrokeDictionary instance;

private:
  static const StenoChord trigger;
};

//---------------------------------------------------------------------------

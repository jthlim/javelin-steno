//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Firmware implementation of https://github.com/jthlim/jeff-numbers.
class StenoJeffNumbersDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffNumbersDictionary() {}

  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const final;
  virtual unsigned int GetMaximumMatchLength() const final { return 10; }
  virtual void PrintInfo() const final;
  virtual bool PrintDictionary(bool hasData) const final { return false; }

  static const StenoJeffNumbersDictionary instance;

private:
  StenoChord GetDigits(char *scratch, StenoChord chord) const;
};

//---------------------------------------------------------------------------

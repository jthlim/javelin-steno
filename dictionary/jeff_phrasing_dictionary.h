//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoJeffPhrasingDictionary final : public StenoDictionary {
public:
  constexpr StenoJeffPhrasingDictionary() {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual unsigned int GetMaximumMatchLength() const { return 1; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const { return false; }
  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  static const StenoJeffPhrasingDictionary instance;

private:
  static size_t CountNumberOfSpaces(const char *p);
  static bool ContainsNonPhraseCharacter(const char *p);

  void RecurseCheckReverseLookup(StenoReverseDictionaryLookup &result,
                                 const char *p, StenoChord stroke,
                                 uint32_t hash, uint8_t componentMask,
                                 uint8_t modeMask,
                                 List<StenoChord> &testedChords) const;
};

//---------------------------------------------------------------------------

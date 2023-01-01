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

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual unsigned int GetMaximumMatchLength() const { return 1; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const { return false; }
  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  static const StenoJeffPhrasingDictionary instance;

private:
  static size_t CountNumberOfSpaces(const char *p);
  static bool ContainsNonPhraseCharacter(const char *p);

  struct ReverseLookupContext;

  void RecurseCheckReverseLookup(ReverseLookupContext &context, const char *p,
                                 StenoChord stroke, uint32_t hash,
                                 uint8_t componentMask, uint8_t modeMask) const;
};

//---------------------------------------------------------------------------

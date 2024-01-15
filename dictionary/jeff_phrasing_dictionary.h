//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "dictionary.h"

//---------------------------------------------------------------------------

struct JeffPhrasingReverseHashMapEntry;

class StenoJeffPhrasingDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 1;

public:
  StenoJeffPhrasingDictionary() : StenoDictionary(MAXIMUM_OUTLINE_LENGTH) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual const char *GetName() const;
  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  static StenoJeffPhrasingDictionary instance;

private:
  static size_t CountNumberOfSpaces(const char *p);
  static bool ContainsNonPhraseCharacter(const char *p);

  struct ReverseLookupContext;

  void RecurseCheckReverseLookup(ReverseLookupContext &context, const char *p,
                                 StenoStroke stroke, uint32_t hash,
                                 uint8_t componentMask, uint8_t modeMask) const;

  void ProcessEntries(const JeffPhrasingReverseHashMapEntry *entry,
                      uint32_t wordHash, ReverseLookupContext &context,
                      const char *p, StenoStroke stroke, uint32_t hash,
                      uint32_t componentMask, uint32_t modeMask) const;
};

//---------------------------------------------------------------------------

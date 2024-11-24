//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoEmilySymbolsDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 1;

public:
  StenoEmilySymbolsDictionary(bool isSpecifySpacesMode)
      : StenoDictionary(MAXIMUM_OUTLINE_LENGTH),
        isSpecifySpacesMode(isSpecifySpacesMode) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

  static StenoEmilySymbolsDictionary specifySpacesInstance;
  static StenoEmilySymbolsDictionary specifyGlueInstance;

private:
  // Whether pressing A or E specifies spaces (vs. specifying glue).
  bool isSpecifySpacesMode;
};

//---------------------------------------------------------------------------

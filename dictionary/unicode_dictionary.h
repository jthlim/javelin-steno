//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Internal dictionary used to handle scan codes in the steno stream.
class StenoUnicodeDictionary final : public StenoDictionary {
public:
  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual size_t GetMaximumOutlineLength() const { return 1; }
  virtual const char *GetName() const;

  static StenoStroke CreateUnicodeStroke(uint32_t unicode) {
    return StenoStroke(StrokeMask::UNICODE | unicode);
  }

  static StenoUnicodeDictionary instance;
};

//---------------------------------------------------------------------------

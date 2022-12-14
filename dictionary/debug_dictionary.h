//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

// Only used for debug test code.
class StenoDebugDictionary final : public StenoDictionary {
public:
  constexpr StenoDebugDictionary() {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual size_t GetMaximumOutlineLength() const { return 1; }
  virtual const char *GetName() const;
  virtual bool PrintDictionary(bool hasData) const { return false; }

  void SetResponse(const char *p) { response = p; }

  static const StenoDebugDictionary instance;

private:
  static const StenoStroke trigger;
  const char *response = nullptr;
};

//---------------------------------------------------------------------------

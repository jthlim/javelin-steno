//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoWrappedDictionary : public StenoDictionary {
private:
  using super = StenoDictionary;

public:
  StenoWrappedDictionary(StenoDictionary *dictionary);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const final;
  using super::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const final;
  using super::GetDictionaryForOutline;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual bool Remove(const char *name, const StenoStroke *strokes,
                      size_t length);

  virtual void SetParentRecursively(StenoDictionary *parent) final;

  virtual const char *GetName() const = 0;

  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

private:
  StenoDictionary *lookupDictionary;
  StenoDictionary *dictionary;

  virtual StenoDictionary *GetWrappedLookupDictionary() const final {
    return lookupDictionary;
  }
};

//---------------------------------------------------------------------------

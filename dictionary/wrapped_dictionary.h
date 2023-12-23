//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoWrappedDictionary : public StenoDictionary {
public:
  StenoWrappedDictionary(StenoDictionary *dictionary)
      : StenoDictionary(dictionary->GetMaximumOutlineLength()),
        dictionary(dictionary) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;

  inline StenoDictionaryLookupResult Lookup(const StenoStroke *strokes,
                                            size_t length) const {
    return Lookup(StenoDictionaryLookup(strokes, length));
  }

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  inline const StenoDictionary *GetLookupProvider(const StenoStroke *strokes,
                                                  size_t length) const {
    return GetLookupProvider(StenoDictionaryLookup(strokes, length));
  }

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual void SetParentRecursively(StenoDictionary *parent) final;

  virtual const char *GetName() const = 0;

  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(const char *name, bool hasData) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

protected:
  StenoDictionary *dictionary;
};

//---------------------------------------------------------------------------

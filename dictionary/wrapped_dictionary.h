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
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  inline const StenoDictionary *
  GetDictionaryForOutline(const StenoStroke *strokes, size_t length) const {
    return GetDictionaryForOutline(StenoDictionaryLookup(strokes, length));
  }

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

protected:
  StenoDictionary *dictionary;
};

//---------------------------------------------------------------------------

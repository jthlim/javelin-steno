//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoWrappedDictionary : public StenoDictionary {
public:
  StenoWrappedDictionary(StenoDictionary *dictionary)
      : dictionary(dictionary) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;

  inline StenoDictionaryLookupResult Lookup(const StenoChord *chords,
                                            size_t length) const {
    return Lookup(StenoDictionaryLookup(chords, length));
  }

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  inline const StenoDictionary *GetLookupProvider(const StenoChord *chords,
                                                  size_t length) const {
    return GetLookupProvider(StenoDictionaryLookup(chords, length));
  }

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual bool
  ReverseMapDictionaryLookup(StenoReverseMapDictionaryLookup &lookup) const;

  virtual unsigned int GetMaximumMatchLength() const;
  virtual const char *GetName() const = 0;

  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

protected:
  StenoDictionary *dictionary;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class StenoReverseMapDictionary final : public StenoDictionary {
public:
  StenoReverseMapDictionary(StenoDictionary *dictionary,
                            const uint8_t *baseAddress,
                            const uint8_t *textBlock, size_t textBlockLength)
      : dictionary(dictionary), baseAddress(baseAddress), textBlock(textBlock),
        textBlockLength(textBlockLength) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual bool ReverseMapDictionaryLookup(StenoReverseDictionaryLookup &result,
                                          const void *data) const;

  virtual unsigned int GetMaximumMatchLength() const;
  virtual const char *GetName() const;

  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

private:
  StenoDictionary *dictionary;
  const uint8_t *baseAddress;
  const uint8_t *textBlock;
  const size_t textBlockLength;

  void AddMapDictionaryResults(StenoReverseDictionaryLookup &result) const;
  void AddValidLookupProviders(StenoReverseDictionaryLookup &result,
                               StenoReverseDictionaryLookup &value) const;
};

//---------------------------------------------------------------------------

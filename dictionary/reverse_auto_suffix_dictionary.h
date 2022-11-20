//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"

//---------------------------------------------------------------------------

class Pattern;
class StenoCompiledOrthography;
class StenoOrthography;
struct StenoOrthographyReverseAutoSuffix;

//---------------------------------------------------------------------------

class StenoReverseAutoSuffixDictionary final : public StenoDictionary {
public:
  StenoReverseAutoSuffixDictionary(StenoDictionary *dictionary,
                                   const StenoCompiledOrthography &orthography);

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
  const StenoCompiledOrthography &orthography;
  const Pattern *reversePatterns;

  static const Pattern *
  CreateReversePatterns(const StenoOrthography &orthography);

  void ProcessReverseAutoSuffix(
      StenoReverseDictionaryLookup &result,
      const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix,
      const Pattern &reversePattern) const;

  bool HasValidLookup(const StenoChord *chords, size_t length) const;
};

//---------------------------------------------------------------------------

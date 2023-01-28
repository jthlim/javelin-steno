//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class Pattern;
class StenoCompiledOrthography;
struct StenoOrthography;
struct StenoOrthographyReverseAutoSuffix;

//---------------------------------------------------------------------------

class StenoReverseAutoSuffixDictionary final : public StenoWrappedDictionary {
public:
  StenoReverseAutoSuffixDictionary(StenoDictionary *dictionary,
                                   const StenoCompiledOrthography &orthography);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual const char *GetName() const;

private:
  const StenoCompiledOrthography &orthography;
  const Pattern *reversePatterns;

  void ProcessReverseAutoSuffix(
      StenoReverseDictionaryLookup &result,
      const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix,
      const Pattern &reversePattern) const;

  bool CanAutoSuffixLookup(const StenoStroke *strokes, size_t length) const;
  bool HasPrefixLookup(const StenoStroke *strokes, size_t length) const;
  bool HasSuffixLookup(const StenoStroke *strokes, size_t length) const;

  static const Pattern *
  CreateReversePatterns(const StenoOrthography &orthography);
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "../pattern_quick_reject.h"
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

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const;

private:
  const StenoCompiledOrthography &orthography;
  const Pattern *reversePatterns;
  PatternQuickReject mergedQuickReject;

  void ProcessReverseAutoSuffix(
      StenoReverseDictionaryLookup &lookup,
      const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix,
      const Pattern &reversePattern) const;

  bool CanAutoSuffixLookup(const StenoStroke *strokes, size_t length) const {
    return !HasValidLookup(strokes, length);
  }
  bool HasValidLookup(const StenoStroke *strokes, size_t length) const;

  static const Pattern *
  CreateReversePatterns(const StenoOrthography &orthography);
};

//---------------------------------------------------------------------------

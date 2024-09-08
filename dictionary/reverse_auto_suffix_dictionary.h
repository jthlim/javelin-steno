//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "../pattern.h"
#include "../pattern_quick_reject.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoCompiledOrthography;
struct StenoOrthography;
struct StenoOrthographyAutoSuffix;
struct StenoOrthographyReverseAutoSuffix;

//---------------------------------------------------------------------------

class StenoReverseAutoSuffixDictionary final : public StenoWrappedDictionary {
private:
  using super = StenoWrappedDictionary;

public:
  StenoReverseAutoSuffixDictionary(StenoDictionary *dictionary,
                                   const StenoCompiledOrthography &orthography);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const;

private:
  const StenoCompiledOrthography &orthography;
  PatternQuickReject mergedQuickReject;

  struct AutoSuffixTest {
    AutoSuffixTest() {}

    const char *testPattern;
    const char *replacement;
    const char *text;
    size_t textLength;
    union {
      Pattern reversePattern;
      size_t _suppressConstructorDummy;
    };
    StaticList<const StenoOrthographyReverseAutoSuffix *, 4> replacements;
  };

  List<AutoSuffixTest> tests;

  void AddTest(const StenoOrthographyReverseAutoSuffix &reverseAutoSuffix);

  void ProcessReverseAutoSuffix(StenoReverseDictionaryLookup &lookup,
                                const AutoSuffixTest &test) const;

  bool CanAutoSuffixLookup(const StenoStroke *strokes, size_t length) const {
    return !HasValidLookup(strokes, length);
  }
  bool HasValidLookup(const StenoStroke *strokes, size_t length) const;
};

//---------------------------------------------------------------------------

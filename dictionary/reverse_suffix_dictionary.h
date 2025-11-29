//---------------------------------------------------------------------------

#pragma once
#include "../container/list.h"
#include "../container/sized_list.h"
#include "../pattern_quick_reject.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class Pattern;
class StenoCompiledOrthography;
struct StenoOrthographyRule;

//---------------------------------------------------------------------------

class StenoReverseSuffixDictionary final : public StenoWrappedDictionary {
private:
  using super = StenoWrappedDictionary;

public:
  StenoReverseSuffixDictionary(
      StenoDictionary *dictionary, const uint8_t *baseAddress,
      const SizedList<StenoOrthographyRule> &reverseSuffixes,
      const StenoCompiledOrthography &orthography,
      const StenoDictionary *prefixDictionary,
      const SizedList<const uint8_t *> suffixes);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual bool IsInternal() const { return true; }
  virtual const char *GetName() const;

private:
  struct Suffix;
  struct ReverseLookupContext;
  struct ReverseSuffix;

  const uint8_t *const baseAddress;
  const SizedList<ReverseSuffix> reversePatterns;
  PatternQuickReject mergedQuickReject;
  const SizedList<Suffix> suffixes;
  const StenoCompiledOrthography &orthography;
  const StenoDictionary *const prefixDictionary;

  void AddSuffixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &lookup) const;

  void AddSuffixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &lookup,
                              const char *withoutSuffix, const Suffix *suffix,
                              size_t suffixLength) const;

  bool IsStrokeDefined(const StenoStroke *strokes, size_t prefixStrokeCount,
                       size_t combinedStrokeCount) const;

  static const SizedList<ReverseSuffix>
  CreateReversePatterns(const SizedList<StenoOrthographyRule> &patterns);
};

//---------------------------------------------------------------------------

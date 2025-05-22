//---------------------------------------------------------------------------

#pragma once
#include "../container/list.h"
#include "../container/sized_list.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoCompiledOrthography;

//---------------------------------------------------------------------------

class StenoReverseSuffixDictionary final : public StenoWrappedDictionary {
private:
  using super = StenoWrappedDictionary;

public:
  StenoReverseSuffixDictionary(StenoDictionary *dictionary,
                               const uint8_t *baseAddress,
                               const StenoCompiledOrthography &orthography,
                               const StenoDictionary *prefixDictionary,
                               const SizedList<const uint8_t *> suffixes,
                               const List<const uint8_t *> &ignoreSuffixes);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;
  virtual const char *GetName() const;

private:
  struct Suffix;
  struct ReverseLookupContext;

  const uint8_t *const baseAddress;
  const SizedList<Suffix> suffixes;
  const StenoCompiledOrthography &orthography;
  const StenoDictionary *const prefixDictionary;

  static SizedList<Suffix>
  CreateSuffixList(const SizedList<const uint8_t *> suffixes,
                   const List<const uint8_t *> &ignoreSuffixes);

  void AddSuffixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &lookup) const;

  bool IsStrokeDefined(const StenoStroke *strokes, size_t prefixStrokeCount,
                       size_t combinedStrokeCount) const;
};

//---------------------------------------------------------------------------

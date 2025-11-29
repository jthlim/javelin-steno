//---------------------------------------------------------------------------

#pragma once
#include "../container/sized_list.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoReversePrefixDictionary final : public StenoWrappedDictionary {
private:
  using super = StenoWrappedDictionary;

public:
  StenoReversePrefixDictionary(StenoDictionary *dictionary,
                               const uint8_t *baseAddress,
                               const SizedList<const uint8_t *> prefixes);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual bool IsInternal() const { return true; }
  virtual const char *GetName() const;

private:
  struct Prefix;
  struct ReverseLookupContext;

  static constexpr size_t MAXIMUM_REVERSE_PREFIX_DEPTH = 2;

  const uint8_t *const baseAddress;
  const SizedList<Prefix> prefixes;

  void AddPrefixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &lookup) const;

  bool IsStrokeDefined(const StenoStroke *strokes, size_t prefixStrokeCount,
                       size_t combinedStrokeCount) const;
};

//---------------------------------------------------------------------------

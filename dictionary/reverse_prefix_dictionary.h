//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoReversePrefixDictionary final : public StenoWrappedDictionary {
public:
  StenoReversePrefixDictionary(StenoDictionary *dictionary,
                               const uint8_t *baseAddress,
                               const uint8_t *textBlock,
                               size_t textBlockLength);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual const char *GetName() const;

  struct Prefix;
  class TextBlockHandler;

private:
  static const size_t MAXIMUM_REVERSE_PREFIX_DEPTH = 2;

  const uint8_t *baseAddress;

  size_t prefixCount;
  const Prefix *prefixes;

  struct ReverseLookupContext;

  void AddPrefixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &result) const;

  static void ProcessTextBlock(const uint8_t *textBlock, size_t textBlockLength,
                               TextBlockHandler &handler);

  bool IsStrokeDefined(const StenoStroke *strokes, size_t prefixStrokeCount,
                       size_t combinedStrokeCount) const;
};

//---------------------------------------------------------------------------

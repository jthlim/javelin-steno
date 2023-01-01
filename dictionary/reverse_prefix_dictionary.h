//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

constexpr size_t PREFIX_NODE_BLOCK_SIZE = 1024;

//---------------------------------------------------------------------------

class StenoReversePrefixDictionary final : public StenoWrappedDictionary {
public:
  StenoReversePrefixDictionary(StenoDictionary *dictionary,
                               const uint8_t *baseAddress,
                               const uint8_t *textBlock,
                               size_t textBlockLength);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual const char *GetName() const;

  class TextBlockHandler;

private:
  const uint8_t *baseAddress;

  size_t prefixCount;
  const uint8_t **prefixes;

  struct ReverseLookupContext;

  void AddPrefixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &result) const;

  static void ProcessTextBlock(const uint8_t *textBlock, size_t textBlockLength,
                               TextBlockHandler &handler);
};

//---------------------------------------------------------------------------

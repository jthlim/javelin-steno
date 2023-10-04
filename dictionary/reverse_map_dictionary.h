//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoReverseMapDictionary final : public StenoWrappedDictionary {
public:
  StenoReverseMapDictionary(StenoDictionary *dictionary,
                            const uint8_t *baseAddress,
                            const uint8_t *textBlock, size_t textBlockLength)
      : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
        textBlock(textBlock), textBlockLength(textBlockLength) {}

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual const char *GetName() const;

private:
  const uint8_t *baseAddress;
  const uint8_t *textBlock;
  const size_t textBlockLength;

  void AddMapDictionaryData(StenoReverseDictionaryLookup &result) const;
};

//---------------------------------------------------------------------------

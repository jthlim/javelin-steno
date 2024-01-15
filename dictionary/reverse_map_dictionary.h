//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoReverseMapDictionary final : public StenoWrappedDictionary {
public:
  StenoReverseMapDictionary(StenoDictionary *dictionary,
                            const uint8_t *baseAddress,
                            const uint8_t *textBlock, size_t textBlockLength);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual const char *GetName() const;

private:
  const uint8_t *baseAddress;
  const uint8_t *textBlock;
  const size_t textBlockLength;

  static const size_t INDEX_SIZE = 128;
  const uint8_t *index[INDEX_SIZE + 1];
  size_t indexSize = 0;

  void AddMapDictionaryData(StenoReverseDictionaryLookup &result) const;
  void FilterResult(StenoReverseDictionaryLookup &result) const;

  void BuildIndex();
};

//---------------------------------------------------------------------------

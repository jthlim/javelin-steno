//---------------------------------------------------------------------------

#pragma once
#include "../sized_list.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoReverseMapDictionary final : public StenoWrappedDictionary {
public:
  StenoReverseMapDictionary(StenoDictionary *dictionary,
                            const uint8_t *baseAddress,
                            const SizedList<uint8_t> &textBlock);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const;

  const uint8_t *FindMapDataLookup(const char *text) const;

private:
  const uint8_t *baseAddress;
  const SizedList<uint8_t> textBlock;

  static const size_t INDEX_SIZE = 128;
  const uint8_t *index[INDEX_SIZE + 1];
  size_t indexSize = 0;

  void AddMapDictionaryData(StenoReverseDictionaryLookup &lookup) const;
  void FilterResult(StenoReverseDictionaryLookup &lookup) const;

  void BuildIndex();
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "../container/sized_list.h"
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoReverseMapDictionary final : public StenoWrappedDictionary {
private:
  using super = StenoWrappedDictionary;

public:
  StenoReverseMapDictionary(StenoDictionary *dictionary,
                            const uint8_t *baseAddress,
                            const SizedList<uint8_t> &textBlock);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const;

  virtual void PrintEntriesWithPrefix(PrintPrefixContext &context) const;

  const uint8_t *FindMapDataLookup(const char *text) const;
  const uint8_t *FindPrefixLookup(const char *text) const;

private:
  const uint8_t *const baseAddress;
  const SizedList<uint8_t> textBlock;

  static constexpr size_t INDEX_SIZE = 256;
  size_t indexSize = 0;
  const uint8_t *index[INDEX_SIZE + 1];

  void AddMapDictionaryData(StenoReverseDictionaryLookup &lookup) const;
  void FilterResult(StenoReverseDictionaryLookup &lookup) const;

  void BuildIndex();
};

//---------------------------------------------------------------------------

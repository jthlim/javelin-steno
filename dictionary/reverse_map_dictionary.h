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

  // Point to the first entry that has the starting letter.
  const uint8_t *index[257];

  void AddMapDictionaryData(StenoReverseDictionaryLookup &lookup) const;
  void FilterResult(StenoReverseDictionaryLookup &lookup) const;

  const uint8_t *FindFirstEntryWithPrefix(int c) const;

  void BuildIndex();
};

//---------------------------------------------------------------------------

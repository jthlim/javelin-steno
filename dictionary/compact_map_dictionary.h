//---------------------------------------------------------------------------

#pragma once
#include "../interval.h"
#include "dictionary.h"

//---------------------------------------------------------------------------

struct CompactStenoMapDictionaryDataEntry;
struct StenoCompactMapDictionaryDefinition;
struct StenoCompactMapDictionaryStrokesDefinition;

//---------------------------------------------------------------------------

class StenoCompactMapDictionary final : public StenoDictionary,
                                        public JavelinMallocAllocate {
public:
  StenoCompactMapDictionary(
      const StenoCompactMapDictionaryDefinition &definition);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual void
  PrintEntriesWithPartialOutline(PrintPartialOutlineContext &context) const;

  virtual void PrintEntriesWithPrefix(PrintPrefixContext &context) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual bool CanRemove() const { return true; }
  virtual bool Remove(const char *dictionaryName, const StenoStroke *strokes,
                      size_t length);

  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

private:
  const uint8_t *const textBlock;
  const StenoCompactMapDictionaryDefinition &definition;
  Interval<const void *> dataRange;

  // This is offset by 1 to simplify lookup code marginally.
  const StenoCompactMapDictionaryStrokesDefinition *const strokes;

  const CompactStenoMapDictionaryDataEntry *
  FindEntry(const StenoDictionaryLookup &lookup) const;

  static const StenoCompactMapDictionaryStrokesDefinition *
  CreateStrokeCache(const StenoCompactMapDictionaryDefinition &definition);
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "../interval.h"
#include "dictionary.h"
#include "dictionary_definition.h"

//---------------------------------------------------------------------------

struct CompactStenoMapDictionaryDataEntry;

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

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual bool CanRemove() const { return true; }
  virtual bool Remove(const char *name, const StenoStroke *strokes,
                      size_t length);

  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

private:
  const uint8_t *textBlock;
  const StenoCompactMapDictionaryDefinition &definition;
  Interval<const void *> dataRange;

  // This is offset by 1 to simplify lookup code marginally.
  const StenoCompactMapDictionaryStrokesDefinition *strokes;

  static const StenoCompactMapDictionaryStrokesDefinition *
  CreateStrokeCache(const StenoCompactMapDictionaryDefinition &definition);

  void ReverseLookup(StenoReverseDictionaryLookup &lookup,
                     const CompactStenoMapDictionaryDataEntry *entry) const;
};

//---------------------------------------------------------------------------

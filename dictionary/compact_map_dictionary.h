//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"
#include "dictionary_definition.h"

//---------------------------------------------------------------------------

class StenoCompactMapDictionary final : public StenoDictionary,
                                        public JavelinMallocAllocate {
public:
  StenoCompactMapDictionary(
      const StenoCompactMapDictionaryDefinition &definition)
      : StenoDictionary(definition.maximumOutlineLength),
        textBlock(definition.textBlock), definition(definition),
        strokes(CreateStrokeCache(definition)) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

private:
  const uint8_t *textBlock;
  const StenoCompactMapDictionaryDefinition &definition;

  // This is offset by 1 to simplify lookup code marginally.
  const StenoCompactMapDictionaryStrokesDefinition *strokes;

  static const StenoCompactMapDictionaryStrokesDefinition *
  CreateStrokeCache(const StenoCompactMapDictionaryDefinition &definition);

  void ReverseLookup(StenoReverseDictionaryLookup &lookup,
                     const void *data) const;
};

//---------------------------------------------------------------------------

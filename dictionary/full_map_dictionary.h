//---------------------------------------------------------------------------

#pragma once
#include "dictionary.h"
#include "map_dictionary_definition.h"

//---------------------------------------------------------------------------

class StenoFullMapDictionary final : public StenoDictionary {
public:
  StenoFullMapDictionary(const StenoMapDictionaryDefinition &definition)
      : StenoDictionary(definition.maximumOutlineLength),
        textBlock(definition.textBlock), definition(definition),
        strokes(CreateStrokeCache(definition)) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  virtual bool
  ReverseMapDictionaryLookup(StenoReverseMapDictionaryLookup &lookup) const;

  virtual size_t GetMaximumOutlineLength() const;
  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const;

private:
  const uint8_t *textBlock;
  const StenoMapDictionaryDefinition &definition;

  // This is offset by 1 to simplify lookup code marginally.
  const StenoMapDictionaryStrokesDefinition *strokes;

  static const StenoMapDictionaryStrokesDefinition *
  CreateStrokeCache(const StenoMapDictionaryDefinition &definition);
};

//---------------------------------------------------------------------------

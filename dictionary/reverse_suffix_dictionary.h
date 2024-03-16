//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoCompiledOrthography;

//---------------------------------------------------------------------------

class StenoReverseSuffixDictionary final : public StenoWrappedDictionary {
public:
  StenoReverseSuffixDictionary(StenoDictionary *dictionary,
                               const uint8_t *baseAddress,
                               const uint8_t *textBlock, size_t textBlockLength,
                               const StenoCompiledOrthography &orthography,
                               const StenoDictionary *prefixDictionary);

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual const char *GetName() const;

  struct Suffix;
  class TextBlockHandler;
  class CountTextBlockHandler;
  class PopulateTextBlockHandler;

private:
  const uint8_t *baseAddress;

  size_t suffixCount;
  const Suffix *suffixes;
  const StenoCompiledOrthography &orthography;
  const StenoDictionary *prefixDictionary;

  struct ReverseLookupContext;

  void AddSuffixReverseLookup(ReverseLookupContext &context,
                              StenoReverseDictionaryLookup &result) const;

  static void ProcessTextBlock(const uint8_t *textBlock, size_t textBlockLength,
                               TextBlockHandler &handler);

  bool IsStrokeDefined(const StenoStroke *strokes, size_t prefixStrokeCount,
                       size_t combinedStrokeCount) const;
};

//---------------------------------------------------------------------------

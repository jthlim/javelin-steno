//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "dictionary.h"
#include "orthospelling_data.h"

//---------------------------------------------------------------------------

class BufferWriter;
struct StenoOrthospellingDictionaryDefinition;

//---------------------------------------------------------------------------

class StenoOrthospellingDictionary final : public StenoDictionary {
private:
  static const size_t MAXIMUM_OUTLINE_LENGTH = 12;

public:
  StenoOrthospellingDictionary(const OrthospellingData &data);
  StenoOrthospellingDictionary(
      const StenoOrthospellingDictionaryDefinition &definition);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual const char *GetName() const;
  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

private:
  const OrthospellingData data;

  void ProcessStroke(BufferWriter &result, OrthospellingData::Context context,
                     StenoStroke stroke) const;
};

//---------------------------------------------------------------------------

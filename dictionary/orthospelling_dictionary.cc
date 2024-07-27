//---------------------------------------------------------------------------

#include "orthospelling_dictionary.h"
#include "../writer.h"
#include "dictionary_definition.h"

//---------------------------------------------------------------------------

StenoOrthospellingDictionary::StenoOrthospellingDictionary(
    const OrthospellingData &data)
    : StenoDictionary(MAXIMUM_OUTLINE_LENGTH), data(data) {}

StenoOrthospellingDictionary::StenoOrthospellingDictionary(
    const StenoOrthospellingDictionaryDefinition &definition)
    : StenoDictionary(definition.maximumOutlineLength), data(definition.data) {}

StenoDictionaryLookupResult StenoOrthospellingDictionary::Lookup(
    const StenoDictionaryLookup &lookup) const {

  const OrthospellingData::Starter *starter =
      data.GetStarterDefinition(lookup.strokes[0]);
  if (starter == nullptr) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  for (size_t i = 1; i < lookup.length; ++i) {
    if (data.GetStarterDefinition(lookup.strokes[i]) != nullptr ||
        data.IsExit(lookup.strokes[i])) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }

  char buffer[64];
  BufferWriter result;
  result.WriteString(starter->definition);

  StenoStroke remainder = lookup.strokes[0] & ~starter->mask;
  if (remainder.IsNotEmpty()) {
    ProcessStroke(result, buffer, remainder);
  }

  for (size_t i = 1; i < lookup.length; ++i) {
    ProcessStroke(result, buffer, lookup.strokes[i]);
  }

  return StenoDictionaryLookupResult::CreateDynamicString(
      result.TerminateStringAndAdoptBuffer());
}

void StenoOrthospellingDictionary::ProcessStroke(BufferWriter &result,
                                                 char (&buffer)[64],
                                                 StenoStroke stroke) const {
  OrthospellingContext context(buffer);
  data.ConvertToText(stroke, context);
  if (*context.buffers[0]) {
    result.WriteString(context.buffers[0]);
  }
  if (*context.buffers[1]) {
    result.WriteString(context.buffers[1]);
  }
  if (*context.buffers[2]) {
    result.WriteString(context.buffers[2]);
  }
  if (*context.buffers[3]) {
    result.WriteString(context.buffers[3]);
  }
}

const StenoDictionary *StenoOrthospellingDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  const OrthospellingData::Starter *starter =
      data.GetStarterDefinition(lookup.strokes[0]);
  return starter != nullptr ? this : nullptr;
}

const char *StenoOrthospellingDictionary::GetName() const { return data.name; }

void StenoOrthospellingDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  // TODO
}

void StenoOrthospellingDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  // Do nothing
}

//---------------------------------------------------------------------------

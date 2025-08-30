//---------------------------------------------------------------------------

#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

StenoWrappedDictionary::StenoWrappedDictionary(StenoDictionary *dictionary)
    : super(dictionary->GetMaximumOutlineLength()), dictionary(dictionary),
      lookupDictionary(dictionary->GetLookupDictionary()) {}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoWrappedDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return lookupDictionary->Lookup(lookup);
}

const StenoDictionary *StenoWrappedDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  return lookupDictionary->GetDictionaryForOutline(lookup);
}

void StenoWrappedDictionary::GetDictionariesForOutline(
    List<const StenoDictionary *> &results,
    const StenoDictionaryLookup &lookup) const {
  return lookupDictionary->GetDictionariesForOutline(results, lookup);
}

void StenoWrappedDictionary::PrintEntriesWithPartialOutline(
    PrintPartialOutlineContext &context) const {
  return lookupDictionary->PrintEntriesWithPartialOutline(context);
}

void StenoWrappedDictionary::PrintEntriesWithPrefix(
    PrintPrefixContext &context) const {
  return dictionary->PrintEntriesWithPrefix(context);
}

//---------------------------------------------------------------------------

void StenoWrappedDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  return dictionary->ReverseLookup(lookup);
}

bool StenoWrappedDictionary::Remove(const char *dictionaryName,
                                    const StenoStroke *strokes, size_t length) {
  return dictionary->Remove(dictionaryName, strokes, length);
}

void StenoWrappedDictionary::SetParentRecursively(StenoDictionary *parent) {
  super::SetParentRecursively(parent);
  dictionary->SetParentRecursively(this);
}

void StenoWrappedDictionary::PrintInfo(int depth) const {
  return dictionary->PrintInfo(depth);
}

void StenoWrappedDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  dictionary->PrintDictionary(context);
}

void StenoWrappedDictionary::ListDictionaries() const {
  return dictionary->ListDictionaries();
}

bool StenoWrappedDictionary::EnableDictionary(const char *name) {
  return dictionary->EnableDictionary(name);
}

bool StenoWrappedDictionary::DisableDictionary(const char *name) {
  return dictionary->DisableDictionary(name);
}

bool StenoWrappedDictionary::ToggleDictionary(const char *name) {
  return dictionary->ToggleDictionary(name);
}

//---------------------------------------------------------------------------

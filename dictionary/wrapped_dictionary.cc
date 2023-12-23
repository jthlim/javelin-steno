//---------------------------------------------------------------------------

#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoWrappedDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return dictionary->Lookup(lookup);
}

const StenoDictionary *StenoWrappedDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  return dictionary->GetLookupProvider(lookup);
}

void StenoWrappedDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  return dictionary->ReverseLookup(result);
}

void StenoWrappedDictionary::SetParentRecursively(StenoDictionary *parent) {
  StenoDictionary::SetParentRecursively(parent);
  dictionary->SetParentRecursively(this);
}

void StenoWrappedDictionary::PrintInfo(int depth) const {
  return dictionary->PrintInfo(depth);
}

bool StenoWrappedDictionary::PrintDictionary(const char *name,
                                             bool hasData) const {
  return dictionary->PrintDictionary(name, hasData);
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

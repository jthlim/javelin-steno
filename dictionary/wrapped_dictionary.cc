//---------------------------------------------------------------------------

#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoWrappedDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  return dictionary->Lookup(lookup);
}

const StenoDictionary *StenoWrappedDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  return dictionary->GetDictionaryForOutline(lookup);
}

void StenoWrappedDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  return dictionary->ReverseLookup(lookup);
}

bool StenoWrappedDictionary::Remove(const char *name,
                                    const StenoStroke *strokes, size_t length) {
  return dictionary->Remove(name, strokes, length);
}

void StenoWrappedDictionary::SetParentRecursively(StenoDictionary *parent) {
  StenoDictionary::SetParentRecursively(parent);
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

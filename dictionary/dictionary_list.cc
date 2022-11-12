//---------------------------------------------------------------------------

#include "dictionary_list.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

StenoDictionaryList::StenoDictionaryList(
    List<StenoDictionaryListEntry> &dictionaries)
    : dictionaries(dictionaries) {
  UpdateMaximumMatchLength();
}

List<StenoDictionaryListEntry> &
CreateList(const StenoDictionary *const *dictionaries, size_t count) {
  List<StenoDictionaryListEntry> *result = new List<StenoDictionaryListEntry>();
  for (size_t i = 0; i < count; ++i) {
    result->Add(StenoDictionaryListEntry(dictionaries[i], true));
  }

  return *result;
}

StenoDictionaryList::StenoDictionaryList(
    const StenoDictionary *const *dictionaries, size_t count)
    : StenoDictionaryList(CreateList(dictionaries, count)) {}

StenoDictionaryLookupResult
StenoDictionaryList::Lookup(const StenoDictionaryLookup &lookup) const {

  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (!dictionaries[i].enabled) {
      continue;
    }
    const StenoDictionary *dictionary = dictionaries[i].dictionary;
    if (dictionary->GetMaximumMatchLength() < lookup.length) {
      continue;
    }

    StenoDictionaryLookupResult result = dictionary->Lookup(lookup);
    if (result.IsValid()) {
      return result;
    }
  }
  return StenoDictionaryLookupResult::CreateInvalid();
}

void StenoDictionaryList::UpdateMaximumMatchLength() {
  unsigned int max = 0;
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (!dictionaries[i].enabled) {
      continue;
    }
    unsigned int m = dictionaries[i].dictionary->GetMaximumMatchLength();
    if (m > max) {
      max = m;
    }
  }
  maximumMatchLength = max;
}

unsigned int StenoDictionaryList::GetMaximumMatchLength() const {
  return maximumMatchLength;
}

const char *StenoDictionaryList::GetName() const { return "list"; }

void StenoDictionaryList::PrintInfo(int depth) const {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    dictionaries[i].dictionary->PrintInfo(depth + 2);
  }
}

bool StenoDictionaryList::PrintDictionary(bool hasData) const {
  // Written in reverse order, so that if there are any conflicts,
  // higher priority items will occur later in the JSON.
  for (size_t i = dictionaries.GetCount(); i != 0;) {
    --i;
    if (!dictionaries[i].enabled) {
      continue;
    }

    bool result = dictionaries[i].dictionary->PrintDictionary(hasData);
    hasData = hasData | result;
  }
  return hasData;
}

//---------------------------------------------------------------------------

void StenoDictionaryList::ListDictionaries() const {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    Console::Printf("%s: %s\n", dictionaries[i].dictionary->GetName(),
                    dictionaries[i].enabled ? "true" : "false");
  }
  Console::Printf("\n");
}

bool StenoDictionaryList::EnableDictionary(const char *name) {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (Str::Eq(name, dictionaries[i].dictionary->GetName())) {
      dictionaries[i].enabled = true;
      Console::Printf("DS %s: true\n\n", name);
      return true;
    }
  }
  return false;
}

bool StenoDictionaryList::DisableDictionary(const char *name) {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (Str::Eq(name, dictionaries[i].dictionary->GetName())) {
      dictionaries[i].enabled = false;
      Console::Printf("DS %s: false\n\n", name);
      return true;
    }
  }
  return false;
}

bool StenoDictionaryList::ToggleDictionary(const char *name) {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (Str::Eq(name, dictionaries[i].dictionary->GetName())) {
      dictionaries[i].enabled = !dictionaries[i].enabled;
      Console::Printf(dictionaries[i].enabled ? "DS %s: true\n\n"
                                              : "DS %s: false\n\n",
                      name);
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------

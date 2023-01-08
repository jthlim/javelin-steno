//---------------------------------------------------------------------------

#include "dictionary_list.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

bool StenoDictionaryList::isSendDictionaryStatusEnabled = false;

//---------------------------------------------------------------------------

StenoDictionaryList::StenoDictionaryList(
    List<StenoDictionaryListEntry> &dictionaries)
    : dictionaries(dictionaries) {
  UpdateMaximumOutlineLength();
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
    if (dictionary->GetMaximumOutlineLength() < lookup.length) {
      continue;
    }

    StenoDictionaryLookupResult result = dictionary->Lookup(lookup);
    if (result.IsValid()) {
      return result;
    }
  }
  return StenoDictionaryLookupResult::CreateInvalid();
}

const StenoDictionary *StenoDictionaryList::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (!dictionaries[i].enabled) {
      continue;
    }
    const StenoDictionary *dictionary = dictionaries[i].dictionary;
    if (dictionary->GetMaximumOutlineLength() < lookup.length) {
      continue;
    }

    const StenoDictionary *result = dictionary->GetLookupProvider(lookup);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

void StenoDictionaryList::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (!dictionaries[i].enabled) {
      continue;
    }
    const StenoDictionary *dictionary = dictionaries[i].dictionary;
    dictionary->ReverseLookup(result);
  }
}

bool StenoDictionaryList::ReverseMapDictionaryLookup(
    StenoReverseMapDictionaryLookup &lookup) const {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (!dictionaries[i].enabled) {
      continue;
    }
    const StenoDictionary *dictionary = dictionaries[i].dictionary;
    if (dictionary->ReverseMapDictionaryLookup(lookup)) {
      StenoDictionaryLookup testLookup(lookup.strokes, lookup.length);
      if (GetLookupProvider(testLookup) == dictionary) {
        return true;
      }
    }
  }
  return false;
}

void StenoDictionaryList::UpdateMaximumOutlineLength() {
  size_t max = 0;
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (!dictionaries[i].enabled) {
      continue;
    }
    size_t m = dictionaries[i].dictionary->GetMaximumOutlineLength();
    if (m > max) {
      max = m;
    }
  }
  maximumOutlineLength = max;
}

size_t StenoDictionaryList::GetMaximumOutlineLength() const {
  return maximumOutlineLength;
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
      SendDictionaryStatus(name, true);
      return true;
    }
  }
  return false;
}

bool StenoDictionaryList::DisableDictionary(const char *name) {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (Str::Eq(name, dictionaries[i].dictionary->GetName())) {
      dictionaries[i].enabled = false;
      SendDictionaryStatus(name, false);
      return true;
    }
  }
  return false;
}

bool StenoDictionaryList::ToggleDictionary(const char *name) {
  for (size_t i = 0; i < dictionaries.GetCount(); ++i) {
    if (Str::Eq(name, dictionaries[i].dictionary->GetName())) {
      dictionaries[i].enabled = !dictionaries[i].enabled;
      SendDictionaryStatus(name, dictionaries[i].enabled);
      return true;
    }
  }
  return false;
}

void StenoDictionaryList::SendDictionaryStatus(const char *name,
                                               bool enabled) const {
  if (isSendDictionaryStatusEnabled) {
    Console::Printf(enabled ? "DS %s: true\n\n" : "DS %s: false\n\n", name);
  }
}

//---------------------------------------------------------------------------

void StenoDictionaryList::EnableDictionaryStatus_Binding(
    void *context, const char *commandLine) {
  EnableSendDictionaryStatus();
  Console::Write("OK\n\n", 4);
}

void StenoDictionaryList::DisableDictionaryStatus_Binding(
    void *context, const char *commandLine) {

  DisableSendDictionaryStatus();
  Console::Write("OK\n\n", 4);
}

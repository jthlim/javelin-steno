//---------------------------------------------------------------------------

#include "dictionary_list.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

bool StenoDictionaryList::isSendDictionaryStatusEnabled = false;

//---------------------------------------------------------------------------

StenoDictionaryList::StenoDictionaryList(
    List<StenoDictionaryListEntry> &dictionaries)
    : super(GetMaximumOutlineLength(dictionaries)), dictionaries(dictionaries) {
  SetParentRecursively(nullptr);
}

List<StenoDictionaryListEntry> &CreateList(StenoDictionary *const *dictionaries,
                                           size_t count) {
  List<StenoDictionaryListEntry> *result = new List<StenoDictionaryListEntry>();
  for (size_t i = 0; i < count; ++i) {
    result->Add(StenoDictionaryListEntry(dictionaries[i], true));
  }

  return *result;
}

StenoDictionaryList::StenoDictionaryList(StenoDictionary *const *dictionaries,
                                         size_t count)
    : StenoDictionaryList(CreateList(dictionaries, count)) {}

StenoDictionaryLookupResult
StenoDictionaryList::Lookup(const StenoDictionaryLookup &lookup) const {
#if ENABLE_DICTIONARY_STATS
  stats.lookupCount++;
#endif
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.combinedMaximumOutlineLength < lookup.length) {
      continue;
    }

    StenoDictionaryLookupResult result = entry->Lookup(lookup);
    if (result.IsValid()) {
      return result;
    }
  }
  return StenoDictionaryLookupResult::CreateInvalid();
}

const StenoDictionary *StenoDictionaryList::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
#if ENABLE_DICTIONARY_STATS
  stats.dictionaryForOutlineCount++;
#endif
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.combinedMaximumOutlineLength < lookup.length) {
      continue;
    }

    if (entry.dictionary == lookup.dictionaryHint) {
      return entry.dictionary;
    }

    const StenoDictionary *result = entry->GetDictionaryForOutline(lookup);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

void StenoDictionaryList::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
#if ENABLE_DICTIONARY_STATS
  stats.reverseLookupCount++;
#endif
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.IsEnabled()) {
      entry->ReverseLookup(lookup);
    }
  }
}

bool StenoDictionaryList::Remove(const char *dictionaryName,
                                 const StenoStroke *strokes, size_t length) {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (Str::Eq(dictionaryName, entry->GetName())) {
      return entry->Remove(dictionaryName, strokes, length);
    }
  }
  return false;
}

void StenoDictionaryList::SetParentRecursively(StenoDictionary *parent) {
  super::SetParentRecursively(parent);
  for (StenoDictionaryListEntry &entry : dictionaries) {
    entry.dictionary->SetParentRecursively(this);
  }
}

void StenoDictionaryList::UpdateMaximumOutlineLength() {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    entry.UpdateMaximumOutlineLength();
  }

  maximumOutlineLength = GetMaximumOutlineLength(dictionaries);
  super::UpdateMaximumOutlineLength();
}

size_t StenoDictionaryList::GetMaximumOutlineLength(
    const FastIterable<StenoDictionaryListEntry> &dictionaries) {
  size_t max = 0;
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.combinedMaximumOutlineLength > max) {
      max = entry.combinedMaximumOutlineLength;
    }
  }
  return max;
}

const char *StenoDictionaryList::GetName() const { return "list"; }

void StenoDictionaryList::PrintInfo(int depth) const {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    entry->PrintInfo(depth + 2);
  }
}

void StenoDictionaryList::PrintDictionary(
    PrintDictionaryContext &context) const {
  // Written in reverse order, so that if there are any conflicts,
  // higher priority items will occur later in the JSON.
  for (const StenoDictionaryListEntry &dictionary : dictionaries.Reverse()) {
    if (dictionary.ShouldPrintDictionary(context.GetName())) {
      dictionary->PrintDictionary(context);
    }
  }
}

//---------------------------------------------------------------------------

void StenoDictionaryList::ListDictionaries() const {
  bool first = true;
  Console::Printf("[\n");
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    const char *name = entry->GetName();
    if (name[0] == '#') {
      continue;
    }
    if (first) {
      first = false;
    } else {
      Console::Printf(",\n");
    }
    Console::Printf(" {\"dictionary\":\"%J\",\"enabled\":%B}", name,
                    entry.IsEnabled());
  }
  Console::Printf("\n]\n\n");
}

bool StenoDictionaryList::EnableDictionary(const char *name) {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    if (Str::Eq(name, entry->GetName())) {
      entry.Enable();
      SendDictionaryStatus(name, true);
      UpdateMaximumOutlineLength();
      return true;
    }
  }
  return false;
}

bool StenoDictionaryList::DisableDictionary(const char *name) {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    if (Str::Eq(name, entry->GetName())) {
      entry.Disable();
      SendDictionaryStatus(name, false);
      UpdateMaximumOutlineLength();
      return true;
    }
  }
  return false;
}

bool StenoDictionaryList::ToggleDictionary(const char *name) {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    if (Str::Eq(name, entry->GetName())) {
      entry.ToggleEnable();
      SendDictionaryStatus(name, entry.IsEnabled());
      UpdateMaximumOutlineLength();
      return true;
    }
  }
  return false;
}

void StenoDictionaryList::SendDictionaryStatus(const char *name,
                                               bool enabled) const {
  if (!isSendDictionaryStatusEnabled) {
    return;
  }

  Console::Printf("EV {"
                  "\"event\":\"dictionary_status\","
                  "\"dictionary\":\"%J\","
                  "\"enabled\":%B"
                  "}\n\n",
                  name, enabled);
}

//---------------------------------------------------------------------------

void StenoDictionaryList::EnableDictionaryStatus_Binding(
    void *context, const char *commandLine) {
  EnableSendDictionaryStatus();
  Console::SendOk();
}

void StenoDictionaryList::DisableDictionaryStatus_Binding(
    void *context, const char *commandLine) {

  DisableSendDictionaryStatus();
  Console::SendOk();
}

//---------------------------------------------------------------------------

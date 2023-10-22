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
  CacheMaximumOutlineLength();
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

const StenoDictionary *StenoDictionaryList::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.combinedMaximumOutlineLength < lookup.length) {
      continue;
    }

    const StenoDictionary *result = entry->GetLookupProvider(lookup);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

void StenoDictionaryList::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (!entry.IsEnabled()) {
      continue;
    }
    entry->ReverseLookup(result);
  }
}

void StenoDictionaryList::CacheMaximumOutlineLength() {
  size_t max = 0;
  for (StenoDictionaryListEntry &entry : dictionaries) {
    ((StenoDictionary *)entry.dictionary)->CacheMaximumOutlineLength();
    size_t m = entry->GetCachedMaximumOutlineLength();
    entry.UpdateMaximumOutlineLength(m);
    if (m > max) {
      max = m;
    }
  }
  cachedMaximumOutlineLength = max;
}

size_t StenoDictionaryList::GetMaximumOutlineLength() const {
  size_t max = 0;
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (!entry.IsEnabled()) {
      continue;
    }
    size_t m = entry->GetMaximumOutlineLength();
    if (m > max) {
      max = m;
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

bool StenoDictionaryList::PrintDictionary(const char *name,
                                          bool hasData) const {
  // Written in reverse order, so that if there are any conflicts,
  // higher priority items will occur later in the JSON.
  for (size_t i = dictionaries.GetCount(); i != 0;) {
    --i;
    if (name) {
      if (!Str::Eq(dictionaries[i]->GetName(), name)) {
        continue;
      }
    } else if (!dictionaries[i].IsEnabled()) {
      continue;
    }

    hasData = dictionaries[i]->PrintDictionary(name, hasData);
  }
  return hasData;
}

//---------------------------------------------------------------------------

void StenoDictionaryList::ListDictionaries() const {
  bool first = true;
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry->GetName()[0] == '#') {
      continue;
    }
    if (first) {
      Console::Printf("[\n");
      first = false;
    } else {
      Console::Printf(",\n");
    }
    Console::Printf(" {\"dictionary\":\"");
    Console::WriteAsJson(entry->GetName());
    Console::Printf("\",\"enabled\":%s}", entry.IsEnabled() ? "true" : "false");
  }
  Console::Printf("\n]\n\n");
}

bool StenoDictionaryList::EnableDictionary(const char *name) {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    if (Str::Eq(name, entry->GetName())) {
      entry.Enable();
      SendDictionaryStatus(name, true);
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

  Console::Printf("EV {\"event\":\"dictionary_status\",\"dictionary\":\"");
  Console::WriteAsJson(name);
  Console::Printf("\",\"enabled\":%s}\n\n", enabled ? "true" : "false");
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

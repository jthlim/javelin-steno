//---------------------------------------------------------------------------

#include "dictionary_list.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

StenoDictionary *
StenoDictionaryList::CreateCacheDictionary(StenoDictionary *dictionary) {
#if ENABLE_DICTIONARY_LOOKUP_CACHE
  return new (cacheDictionaryContainer) StenoCacheDictionary(dictionary);
#else
  return dictionary;
#endif
}

//---------------------------------------------------------------------------

StenoDictionaryList::StenoDictionaryList(
    List<StenoDictionaryListEntry> &&dictionaries)
    : super(GetMaximumOutlineLength(dictionaries)),
      dictionaries((List<StenoDictionaryListEntry> &&)dictionaries) {
  SetParentRecursively(nullptr);
}

List<StenoDictionaryListEntry> CreateList(StenoDictionary *const *dictionaries,
                                          size_t count) {
  List<StenoDictionaryListEntry> result;
  for (size_t i = 0; i < count; ++i) {
    result.Add(StenoDictionaryListEntry(dictionaries[i], true));
  }

  return result;
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
#if ENABLE_DICTIONARY_LOOKUP_CACHE
      if (lookup.updateCache) {
        cacheDictionaryContainer->AddResult(lookup, result, entry.dictionary);
      }
#endif
      return result;
    }
  }
#if ENABLE_DICTIONARY_LOOKUP_CACHE
  if (lookup.updateCache) {
    cacheDictionaryContainer->AddNoResult(lookup);
  }
#endif
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
#if ENABLE_DICTIONARY_LOOKUP_CACHE
      if (lookup.updateCache) {
        cacheDictionaryContainer->AddResult(
            lookup, StenoDictionaryLookupResult::CreateInvalid(),
            entry.dictionary);
      }
#endif
      return result;
    }
  }
#if ENABLE_DICTIONARY_LOOKUP_CACHE
  if (lookup.updateCache) {
    cacheDictionaryContainer->AddNoResult(lookup);
  }
#endif
  return nullptr;
}

void StenoDictionaryList::GetDictionariesForOutline(
    List<const StenoDictionary *> &results,
    const StenoDictionaryLookup &lookup) const {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.combinedMaximumOutlineLength < lookup.length) {
      continue;
    }

    entry->GetDictionariesForOutline(results, lookup);
  }
}

void StenoDictionaryList::PrintEntriesWithPartialOutline(
    PrintPartialOutlineContext &context) const {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    if (entry.combinedMaximumOutlineLength <= context.length) {
      continue;
    }

    entry->PrintEntriesWithPartialOutline(context);
    if (context.IsDone()) {
      return;
    }
  }
}

void StenoDictionaryList::PrintEntriesWithPrefix(
    PrintPrefixContext &context) const {
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    entry->PrintEntriesWithPrefix(context);
    if (context.IsDone()) {
      return;
    }
  }
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

void StenoDictionaryList::OnLookupDataChanged() {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    entry.UpdateMaximumOutlineLength();
  }

  maximumOutlineLength = GetMaximumOutlineLength(dictionaries);
  super::OnLookupDataChanged();
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
  bool isFirst = true;
  Console::Printf("[");
  for (const StenoDictionaryListEntry &entry : dictionaries) {
    const char *name = entry->GetName();
    if (name[0] == '#') {
      continue;
    }
    const char *format = ",\n{\"d\":\"%J\",\"v\":%d}";
    Console::Printf(format + isFirst, name, entry.IsEnabled());
    isFirst = false;
  }
  Console::Printf("\n]\n\n");
}

bool StenoDictionaryList::EnableDictionary(const char *name) {
  for (StenoDictionaryListEntry &entry : dictionaries) {
    if (Str::Eq(name, entry->GetName())) {
      entry.Enable();
      SendDictionaryStatus(name, true);
      OnLookupDataChanged();
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
      OnLookupDataChanged();
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
      OnLookupDataChanged();
      return true;
    }
  }
  return false;
}

void StenoDictionaryList::SendDictionaryStatus(const char *name,
                                               bool enabled) const {
  if (!Console::IsEventEnabled(ConsoleEvent::DICTIONARY_STATUS)) {
    return;
  }

  Console::Printf("EV {\"e\":\"d\",\"d\":\"%J\",\"v\":%d}\n\n", name, enabled);
}

//---------------------------------------------------------------------------

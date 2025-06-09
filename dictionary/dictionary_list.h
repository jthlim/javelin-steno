//---------------------------------------------------------------------------

#pragma once
#include "../container/fast_iterable.h"
#include "../container/list.h"
#include "dictionary.h"

//---------------------------------------------------------------------------

struct StenoDictionaryListEntry {
  StenoDictionaryListEntry(StenoDictionary *dictionary, bool enabled)
      : enabled(enabled),
        combinedMaximumOutlineLength(
            enabled ? dictionary->GetMaximumOutlineLength() : 0),
        dictionary(dictionary) {}

  bool enabled;
  uint16_t combinedMaximumOutlineLength;
  StenoDictionary *dictionary;

  StenoDictionary *operator->() const { return dictionary; }

  bool IsEnabled() const { return enabled; }

  void Enable() {
    enabled = true;
    combinedMaximumOutlineLength = dictionary->GetMaximumOutlineLength();
  }

  void Disable() {
    enabled = false;
    combinedMaximumOutlineLength = 0;
  }

  void ToggleEnable() {
    if (IsEnabled()) {
      Disable();
    } else {
      Enable();
    }
  }

  void UpdateMaximumOutlineLength() {
    if (enabled) {
      combinedMaximumOutlineLength = dictionary->GetMaximumOutlineLength();
    }
  }

  bool ShouldPrintDictionary(const char *name) const {
    return name ? Str::Eq(dictionary->GetName(), name) : IsEnabled();
  }
};

//---------------------------------------------------------------------------

class StenoDictionaryList final : public StenoDictionary {
private:
  using super = StenoDictionary;

public:
  StenoDictionaryList(List<StenoDictionaryListEntry> &dictionaries);
  StenoDictionaryList(StenoDictionary *const *dictionaries, size_t count);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual void
  GetDictionariesForOutline(List<const StenoDictionary *> &results,
                            const StenoDictionaryLookup &lookup) const;

  virtual void
  PrintEntriesWithPartialOutline(PrintPartialOutlineContext &context) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  virtual bool Remove(const char *dictionaryName, const StenoStroke *strokes,
                      size_t length);

  virtual void SetParentRecursively(StenoDictionary *parent);

  virtual void UpdateMaximumOutlineLength();

  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

private:
  FastIterable<StenoDictionaryListEntry> dictionaries;

  void SendDictionaryStatus(const char *name, bool enabled) const;

  static size_t GetMaximumOutlineLength(
      const FastIterable<StenoDictionaryListEntry> &dictionaries);
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "dictionary.h"

//---------------------------------------------------------------------------

struct StenoDictionaryListEntry {
  StenoDictionaryListEntry(StenoDictionary *dictionary, bool enabled)
      : enabled(enabled),
        combinedMaximumOutlineLength(
            enabled ? dictionary->GetMaximumOutlineLength() : 0),
        dictionary(dictionary) {}

  bool enabled;
  size_t combinedMaximumOutlineLength;
  StenoDictionary *dictionary;

  StenoDictionary *operator->() const { return dictionary; }

  void Enable() {
    enabled = true;
    combinedMaximumOutlineLength = dictionary->GetMaximumOutlineLength();
  }
  void Disable() {
    enabled = false;
    combinedMaximumOutlineLength = 0;
  }
  bool IsEnabled() const { return enabled; }
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
};

//---------------------------------------------------------------------------

class StenoDictionaryList final : public StenoDictionary {
public:
  StenoDictionaryList(List<StenoDictionaryListEntry> &dictionaries);
  StenoDictionaryList(StenoDictionary *const *dictionaries, size_t count);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;

  virtual void SetParentRecursively(StenoDictionary *parent);

  virtual void UpdateMaximumOutlineLength();

  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(const char *name, bool hasData) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

  static void EnableSendDictionaryStatus() {
    isSendDictionaryStatusEnabled = true;
  }
  static void DisableSendDictionaryStatus() {
    isSendDictionaryStatusEnabled = false;
  }

  static void EnableDictionaryStatus_Binding(void *context,
                                             const char *commandLine);
  static void DisableDictionaryStatus_Binding(void *context,
                                              const char *commandLine);

private:
  List<StenoDictionaryListEntry> &dictionaries;

  static bool isSendDictionaryStatusEnabled;

  void SendDictionaryStatus(const char *name, bool enabled) const;

  static size_t
  GetMaximumOutlineLength(const List<StenoDictionaryListEntry> &dictionaries);
};

//---------------------------------------------------------------------------

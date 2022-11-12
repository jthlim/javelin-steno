//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "dictionary.h"

//---------------------------------------------------------------------------

struct StenoDictionaryListEntry {
  StenoDictionaryListEntry(const StenoDictionary *dictionary, bool enabled)
      : enabled(enabled), dictionary(dictionary) {}

  bool enabled;
  const StenoDictionary *dictionary;
};

//---------------------------------------------------------------------------

class StenoDictionaryList final : public StenoDictionary {
public:
  StenoDictionaryList(List<StenoDictionaryListEntry> &dictionaries);
  StenoDictionaryList(const StenoDictionary *const *dictionaries, size_t count);

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;

  virtual unsigned int GetMaximumMatchLength() const;
  virtual const char *GetName() const;
  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const;

  virtual void ListDictionaries() const;
  virtual bool EnableDictionary(const char *name);
  virtual bool DisableDictionary(const char *name);
  virtual bool ToggleDictionary(const char *name);

private:
  List<StenoDictionaryListEntry> &dictionaries;
  uint32_t maximumMatchLength;

  void UpdateMaximumMatchLength();
};

//---------------------------------------------------------------------------

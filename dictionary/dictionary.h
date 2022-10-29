//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class StenoChord;

//---------------------------------------------------------------------------

// A class to wrap dictionary lookups, avoiding memory allocations in most
// situations.
struct StenoDictionaryLookup {
  bool IsValid() const { return GetTextMethod != nullptr; }

  const char *GetText() const { return GetTextMethod(this); }
  void Destroy() {
    if (DestroyMethod != nullptr) {
      DestroyMethod(this);
    }
  }

  const char *(*GetTextMethod)(const StenoDictionaryLookup *);
  void (*DestroyMethod)(StenoDictionaryLookup *);
  const void *context;

  static StenoDictionaryLookup CreateInvalid() {
    StenoDictionaryLookup result = {
        .GetTextMethod = nullptr,
        .DestroyMethod = nullptr,
    };
    return result;
  }

  static StenoDictionaryLookup CreateStaticString(const uint8_t *p) {
    return CreateStaticString((const char *)p);
  }
  static StenoDictionaryLookup CreateStaticString(const char *p);

  // string will be free() when the Lookup is destroyed.
  static StenoDictionaryLookup CreateDynamicString(const uint8_t *p) {
    return CreateDynamicString((const char *)p);
  }
  static StenoDictionaryLookup CreateDynamicString(const char *p);
};

//---------------------------------------------------------------------------

class StenoDictionary {
public:
  virtual StenoDictionaryLookup Lookup(const StenoChord *chords,
                                       size_t length) const = 0;
  virtual unsigned int GetMaximumMatchLength() const = 0;
  virtual void PrintInfo() const = 0;
  virtual bool PrintDictionary(bool hasData) const = 0;
};

//---------------------------------------------------------------------------

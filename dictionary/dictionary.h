//---------------------------------------------------------------------------

#pragma once
#include "../chord.h"
#include "../str.h"
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

// A class to wrap dictionary lookups, avoiding memory allocations in most
// situations.
struct StenoDictionaryLookupResult {
  bool IsValid() const { return GetTextMethod != nullptr; }

  const char *GetText() const { return GetTextMethod(this); }
  void Destroy() {
    if (DestroyMethod != nullptr) {
      DestroyMethod(this);
    }
  }

  const char *(*GetTextMethod)(const StenoDictionaryLookupResult *);
  void (*DestroyMethod)(StenoDictionaryLookupResult *);
  const void *context;

  static StenoDictionaryLookupResult CreateInvalid() {
    StenoDictionaryLookupResult result = {
        .GetTextMethod = nullptr,
        .DestroyMethod = nullptr,
    };
    return result;
  }

  static StenoDictionaryLookupResult CreateStaticString(const uint8_t *p) {
    return CreateStaticString((const char *)p);
  }
  static StenoDictionaryLookupResult CreateStaticString(const char *p);

  // string will be free() when the Lookup is destroyed.
  static StenoDictionaryLookupResult CreateDynamicString(const uint8_t *p) {
    return CreateDynamicString((const char *)p);
  }
  static StenoDictionaryLookupResult CreateDynamicString(const char *p);
};

//---------------------------------------------------------------------------

struct StenoDictionaryLookup {
  StenoDictionaryLookup(const StenoChord *chords, size_t length)
      : chords(chords), length(length), hash(StenoChord::Hash(chords, length)) {
  }

  const StenoChord *chords;
  size_t length;
  uint32_t hash;
};

//---------------------------------------------------------------------------

class StenoReverseDictionaryLookup {
public:
  StenoReverseDictionaryLookup(size_t strokeThreshold, const char *lookup)
      : strokeThreshold(strokeThreshold), lookup(lookup),
        lookupLength(strlen(lookup)) {}

  void AddResult(const StenoChord *chords, size_t length);
  bool HasResult(const StenoChord *chords, size_t length) const;

  // Results equal to, or above this will not be captured.
  size_t strokeThreshold;
  const char *lookup;
  size_t lookupLength;

  size_t resultCount = 0;
  uint8_t resultLengths[24];

  size_t chordsCount = 0;
  static const size_t CHORD_COUNT = 64;
  StenoChord chords[CHORD_COUNT];

  static const size_t MAX_STROKE_THRESHOLD = 31;
};

//---------------------------------------------------------------------------

class StenoDictionary {
public:
  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const = 0;

  inline StenoDictionaryLookupResult Lookup(const StenoChord *chords,
                                            size_t length) const {
    return Lookup(StenoDictionaryLookup(chords, length));
  }

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual bool ReverseMapDictionaryLookup(StenoReverseDictionaryLookup &result,
                                          const void *data) const;

  virtual unsigned int GetMaximumMatchLength() const = 0;
  virtual const char *GetName() const = 0;

  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const = 0;

  virtual void ListDictionaries() const {}
  virtual bool EnableDictionary(const char *name) { return false; }
  virtual bool DisableDictionary(const char *name) { return false; }
  virtual bool ToggleDictionary(const char *name) { return false; }

protected:
  static const char *Spaces(int count) { return SPACES + SPACES_COUNT - count; }

private:
  static const size_t SPACES_COUNT = 16;
  static const char SPACES[];
};

//---------------------------------------------------------------------------

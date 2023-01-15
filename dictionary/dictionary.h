//---------------------------------------------------------------------------

#pragma once
#include "../str.h"
#include "../stroke.h"
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class StenoDictionary;

//---------------------------------------------------------------------------

// A class to wrap dictionary lookups, avoiding memory allocations in most
// situations.
class StenoDictionaryLookupResult {
private:
  struct StenoDictionaryLookupResultVtbl {
    const char *(*getTextMethod)(const StenoDictionaryLookupResult *);
    void (*destroyMethod)(StenoDictionaryLookupResult *);
  };

  static const StenoDictionaryLookupResultVtbl invalidVtbl;
  static const StenoDictionaryLookupResultVtbl staticVtbl;
  static const StenoDictionaryLookupResultVtbl dynamicVtbl;

public:
  bool IsValid() const { return vtbl->getTextMethod != nullptr; }

  const char *GetText() const { return vtbl->getTextMethod(this); }
  void Destroy() { vtbl->destroyMethod(this); }

  const StenoDictionaryLookupResultVtbl *vtbl;
  const void *context;

  static StenoDictionaryLookupResult CreateInvalid() {
    StenoDictionaryLookupResult result;
    result.vtbl = &invalidVtbl;
    return result;
  }

  static StenoDictionaryLookupResult CreateStaticString(const uint8_t *p) {
    return CreateStaticString((const char *)p);
  }

  static StenoDictionaryLookupResult CreateStaticString(const char *p) {
    StenoDictionaryLookupResult result;
    result.vtbl = &staticVtbl;
    result.context = p;
    return result;
  }

  // string will be free() when the Lookup is destroyed.
  static StenoDictionaryLookupResult CreateDynamicString(const uint8_t *p) {
    return CreateDynamicString((const char *)p);
  }
  static StenoDictionaryLookupResult CreateDynamicString(const char *p) {
    StenoDictionaryLookupResult result;
    result.vtbl = &dynamicVtbl;
    result.context = p;
    return result;
  }
};

//---------------------------------------------------------------------------

struct StenoDictionaryLookup {
  StenoDictionaryLookup(const StenoStroke *strokes, size_t length)
      : strokes(strokes), length(length),
        hash(StenoStroke::Hash(strokes, length)) {}

  const StenoStroke *strokes;
  size_t length;
  uint32_t hash;
};

//---------------------------------------------------------------------------

struct StenoReverseDictionaryResult {
  size_t length;
  StenoStroke *strokes;
  const StenoDictionary *lookupProvider;
};

class StenoReverseDictionaryLookup {
public:
  StenoReverseDictionaryLookup(size_t strokeThreshold, const char *lookup)
      : strokeThreshold(strokeThreshold), lookup(lookup),
        lookupLength(strlen(lookup)) {}

  void AddResult(const StenoStroke *strokes, size_t length,
                 const StenoDictionary *lookupProvider);
  bool HasResult(const StenoStroke *strokes, size_t length) const;

  // Results equal to, or above this will not be captured.
  size_t strokeThreshold;
  const char *lookup;
  size_t lookupLength;

  size_t resultCount = 0;
  size_t strokesCount = 0;

  StenoReverseDictionaryResult results[24];

  static const size_t STROKE_COUNT = 64;
  StenoStroke strokes[STROKE_COUNT];

  static const size_t MAX_STROKE_THRESHOLD = 31;
};

//---------------------------------------------------------------------------

struct StenoReverseMapDictionaryLookup {
  StenoReverseMapDictionaryLookup(const void *data) : data(data) {}

  const void *data;
  const StenoDictionary *provider;
  size_t length;
  StenoStroke strokes[32];
};

//---------------------------------------------------------------------------

class StenoDictionary {
public:
  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const = 0;

  inline StenoDictionaryLookupResult Lookup(const StenoStroke *strokes,
                                            size_t length) const {
    return Lookup(StenoDictionaryLookup(strokes, length));
  }

  virtual const StenoDictionary *
  GetLookupProvider(const StenoDictionaryLookup &lookup) const;

  inline const StenoDictionary *GetLookupProvider(const StenoStroke *strokes,
                                                  size_t length) const {
    return GetLookupProvider(StenoDictionaryLookup(strokes, length));
  }

  virtual void ReverseLookup(StenoReverseDictionaryLookup &result) const;
  virtual bool
  ReverseMapDictionaryLookup(StenoReverseMapDictionaryLookup &lookup) const;

  virtual size_t GetMaximumOutlineLength() const = 0;
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

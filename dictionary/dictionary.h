//---------------------------------------------------------------------------

#pragma once
#include "../str.h"
#include "../stroke.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class StenoDictionary;

//---------------------------------------------------------------------------

// A class to wrap dictionary lookups, avoiding memory allocations in most
// situations.

#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK

// An implementation that requires that the top bit of the address is never
// used, and packs the entire result into a register.
class StenoDictionaryLookupResult {
private:
  StenoDictionaryLookupResult(size_t text) : text(text) {}

  size_t text;

public:
  bool IsValid() const { return text != 0; }

  const char *GetText() const { return (char *)(text >> 1); }
  void Destroy();

  static StenoDictionaryLookupResult CreateInvalid() {
    return StenoDictionaryLookupResult(0);
  }

  static StenoDictionaryLookupResult CreateStaticString(const uint8_t *p) {
    return CreateStaticString((const char *)p);
  }

  static StenoDictionaryLookupResult CreateStaticString(const char *p) {
    return StenoDictionaryLookupResult(intptr_t(p) << 1);
  }

  // string will be free() when the Lookup is destroyed.
  static StenoDictionaryLookupResult CreateDynamicString(const uint8_t *p) {
    return CreateDynamicString((const char *)p);
  }
  static StenoDictionaryLookupResult CreateDynamicString(const char *p) {
    return StenoDictionaryLookupResult((intptr_t(p) << 1) + 1);
  }
};
#else
class StenoDictionaryLookupResult {
private:
  const char *text;
  void (*destroyMethod)(StenoDictionaryLookupResult *);

  static void Nop(StenoDictionaryLookupResult *);
  static void FreeText(StenoDictionaryLookupResult *);

public:
  bool IsValid() const { return text != nullptr; }

  const char *GetText() const { return text; }
  void Destroy() {
    if (text) {
      (*destroyMethod)(this);
    }
  }

  static StenoDictionaryLookupResult CreateInvalid() {
    StenoDictionaryLookupResult result;
    result.text = nullptr;
    return result;
  }

  static StenoDictionaryLookupResult CreateStaticString(const uint8_t *p) {
    return CreateStaticString((const char *)p);
  }

  static StenoDictionaryLookupResult CreateStaticString(const char *p) {
    StenoDictionaryLookupResult result;
    result.text = p;
    result.destroyMethod = &Nop;
    return result;
  }

  // string will be free() when the Lookup is destroyed.
  static StenoDictionaryLookupResult CreateDynamicString(const uint8_t *p) {
    return CreateDynamicString((const char *)p);
  }
  static StenoDictionaryLookupResult CreateDynamicString(const char *p) {
    StenoDictionaryLookupResult result;
    result.text = p;
    result.destroyMethod = &FreeText;
    return result;
  }
};
#endif

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

  size_t GetCachedMaximumOutlineLength() const {
    return cachedMaximumOutlineLength;
  }
  virtual void CacheMaximumOutlineLength() {
    cachedMaximumOutlineLength = GetMaximumOutlineLength();
  }

  virtual size_t GetMaximumOutlineLength() const = 0;
  virtual const char *GetName() const = 0;

  virtual void PrintInfo(int depth) const;
  virtual bool PrintDictionary(bool hasData) const { return hasData; }

  virtual void ListDictionaries() const {}
  virtual bool EnableDictionary(const char *name) { return false; }
  virtual bool DisableDictionary(const char *name) { return false; }
  virtual bool ToggleDictionary(const char *name) { return false; }

protected:
  StenoDictionary() = default;

  constexpr StenoDictionary(size_t cachedMaximumOutlineLength)
      : cachedMaximumOutlineLength(cachedMaximumOutlineLength) {}

  size_t cachedMaximumOutlineLength;
  static const char *Spaces(int count) { return SPACES + SPACES_COUNT - count; }

private:
  static const size_t SPACES_COUNT = 16;
  static const char SPACES[];
};

//---------------------------------------------------------------------------

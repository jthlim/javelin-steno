//---------------------------------------------------------------------------

#pragma once
#include "../crc.h"
#include "../malloc_allocate.h"
#include "../static_list.h"
#include "../str.h"
#include "../stroke.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class MapDataLookup;
class StenoDictionary;

//---------------------------------------------------------------------------

// A class to wrap dictionary lookups, avoiding memory allocations in most
// situations.

#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK

// An implementation that requires that the top bit of the address is never
// used, and packs the entire result into a register.
//
// Invalid results are represented with 0
// Static strings are represented by (p << 1) -- lowest bit is zero
// Dynamic strings are represented by (p << 1) +1 -- lowest bit is one.
class StenoDictionaryLookupResult {
private:
  StenoDictionaryLookupResult(size_t text) : text(text) {}

  size_t text;

public:
  bool IsValid() const { return text != 0; }

  const char *GetText() const { return (char *)(text >> 1); }
  void Destroy();

  StenoDictionaryLookupResult Clone() const;

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

  bool operator==(const StenoDictionaryLookupResult &other) const {
    return text == other.text;
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

  StenoDictionaryLookupResult Clone() const;

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

  bool operator==(const StenoDictionaryLookupResult &other) const {
    return text == other.text;
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
  const StenoDictionary *dictionary;
};

class StenoReverseDictionaryLookup : public JavelinMallocAllocate {
private:
  static const size_t MAX_MAP_DATA_LOOKUP_COUNT = 24;

public:
  StenoReverseDictionaryLookup(size_t strokeThreshold, const char *definition)
      : strokeThreshold(strokeThreshold), definition(definition),
        definitionLength(strlen(definition)) {}

  bool HasResults() const { return results.IsNotEmpty(); }

  void AddResult(const StenoStroke *strokes, size_t length,
                 const StenoDictionary *dictionary);
  bool HasResult(const StenoStroke *strokes, size_t length) const;

  size_t GetMinimumStrokeCount() const;

  // Results equal to, or above this will not be captured.
  // i.e. Only stroke count less than this will be returned.
  size_t strokeThreshold;
  const char *definition;
  size_t definitionLength;

  size_t strokesCount = 0;

  // Used to prevent recursing prefixes too far.
  size_t prefixLookupDepth = 0;

  // These are used as an optimization for map lookup.
  // Since the first step of all map lookups is the same, do it once and
  // pass it down
  StaticList<const void *, MAX_MAP_DATA_LOOKUP_COUNT> mapDataLookups;

  void AddMapDataLookup(MapDataLookup mapDataLookup,
                        const uint8_t *baseAddress);

  void SortResults();

  StaticList<StenoReverseDictionaryResult, 24> results;

  static const size_t STROKE_COUNT = 64;
  StenoStroke strokes[STROKE_COUNT];

  static const size_t MAX_STROKE_THRESHOLD = 31;

  uint32_t GetLookupCrc() {
    if (!hasLookupCrc) {
      hasLookupCrc = true;
      lookupCrc = Crc32(definition, definitionLength);
    }
    return lookupCrc;
  }

  bool AreAllFromSameDictionary() const;

private:
  bool hasLookupCrc = false;
  uint32_t lookupCrc;
};

//---------------------------------------------------------------------------

class PrintDictionaryContext {
public:
  PrintDictionaryContext(const char *name) : name(name) {}

  bool HasName() const { return name != nullptr; }
  const char *GetName() const { return name; }

  void Print(const StenoStroke *strokes, size_t length, const char *definition);

  void Print(const StenoStroke &stroke, const char *definition) {
    Print(&stroke, 1, definition);
  }

private:
  bool hasData = false;
  const char *name;
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
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

  inline const StenoDictionary *
  GetDictionaryForOutline(const StenoStroke *strokes, size_t length) const {
    return GetDictionaryForOutline(StenoDictionaryLookup(strokes, length));
  }
  inline bool HasOutline(const StenoStroke *strokes, size_t length) const {
    return GetDictionaryForOutline(strokes, length) != nullptr;
  }

  virtual bool CanRemove() const { return false; }
  virtual bool Remove(const char *name, const StenoStroke *strokes,
                      size_t length) {
    return false;
  }

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  size_t GetMaximumOutlineLength() const { return maximumOutlineLength; }
  virtual void UpdateMaximumOutlineLength() {
    if (parent) {
      parent->UpdateMaximumOutlineLength();
    }
  }

  virtual void SetParentRecursively(StenoDictionary *parent) {
    this->parent = parent;
  }

  virtual const char *GetName() const = 0;

  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const {}

  virtual void ListDictionaries() const {}
  virtual bool EnableDictionary(const char *name) { return false; }
  virtual bool DisableDictionary(const char *name) { return false; }
  virtual bool ToggleDictionary(const char *name) { return false; }

protected:
  StenoDictionary(size_t maximumOutlineLength)
      : maximumOutlineLength(maximumOutlineLength), parent(nullptr) {}

  size_t maximumOutlineLength;
  StenoDictionary *parent;

  static const char *Spaces(int count) { return SPACES + SPACES_COUNT - count; }

private:
  static const size_t SPACES_COUNT = 16;
  static const char SPACES[];
};

//---------------------------------------------------------------------------

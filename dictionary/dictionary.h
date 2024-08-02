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

#define ENABLE_DICTIONARY_STATS 0

//---------------------------------------------------------------------------

class BufferWriter;
class MapDataLookup;
class StenoDictionary;

//---------------------------------------------------------------------------

// A class to wrap dictionary lookups, avoiding memory allocations in most
// situations.

#if JAVELIN_PLATFORM_NRF5_SDK || JAVELIN_PLATFORM_PICO_SDK
// An implementation that requires that the RAM region is identifiable
// from the pointer value.
//
// Invalid results are represented with 0
// Static strings are represented by a pointer to ROM region
// Dynamic strings are represented by a pointer to RAM region
class StenoDictionaryLookupResult {
private:
  constexpr StenoDictionaryLookupResult(const char *text) : text(text) {}

  const char *text;

  static bool IsStatic(const void *p) {
    return (intptr_t(p) & 0x20000000) == 0;
  }

  static void DestroyInternal(const char *text);
  static const char *CloneInternal(const char *text);

public:
  bool IsValid() const { return text != nullptr; }

  const char *GetText() const { return text; }
  void Destroy() { DestroyInternal(text); }

  static const StenoDictionaryLookupResult NO_OP;

  StenoDictionaryLookupResult Clone() const {
    return StenoDictionaryLookupResult(CloneInternal(text));
  }

  static StenoDictionaryLookupResult CreateInvalid() {
    return StenoDictionaryLookupResult(nullptr);
  }

  static StenoDictionaryLookupResult CreateStaticString(const uint8_t *p) {
    return CreateStaticString((const char *)p);
  }

  static StenoDictionaryLookupResult CreateStaticString(const char *p) {
    return StenoDictionaryLookupResult(p);
  }

  // string will be free() when the Lookup is destroyed.
  static StenoDictionaryLookupResult CreateDynamicString(const uint8_t *p) {
    return CreateDynamicString((const char *)p);
  }
  static StenoDictionaryLookupResult CreateDynamicString(const char *p) {
    return StenoDictionaryLookupResult(p);
  }

  static StenoDictionaryLookupResult CreateDup(const char *p) {
    return CreateDynamicString(Str::Dup(p));
  }

  static StenoDictionaryLookupResult CreateDupN(const char *p, size_t n) {
    return CreateDynamicString(Str::DupN(p, n));
  }

  static StenoDictionaryLookupResult CreateFromBuffer(BufferWriter &writer);

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

  static const StenoDictionaryLookupResult NO_OP;

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

  static StenoDictionaryLookupResult CreateDup(const char *p) {
    return CreateDynamicString(Str::Dup(p));
  }

  static StenoDictionaryLookupResult CreateDupN(const char *p, size_t n) {
    return CreateDynamicString(Str::DupN(p, n));
  }

  static StenoDictionaryLookupResult CreateFromBuffer(BufferWriter &writer);

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

  StenoDictionaryLookup(const StenoStroke *strokes, size_t length,
                        const StenoDictionary *dictionaryHint)
      : strokes(strokes), length(length),
        hash(StenoStroke::Hash(strokes, length)),
        dictionaryHint(dictionaryHint) {}

  const StenoStroke *strokes;
  size_t length;
  uint32_t hash;
  const StenoDictionary *dictionaryHint;
};

//---------------------------------------------------------------------------

struct StenoReverseDictionaryResult {
  size_t length;
  StenoStroke *strokes;
  const StenoDictionary *dictionary;
};

class StenoReverseDictionaryLookup : public JavelinMallocAllocate {
public:
  StenoReverseDictionaryLookup(const char *definition,
                               size_t strokeThreshold = MAX_STROKE_THRESHOLD)
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

  // Used to prevent recursing prefixes too far.
  size_t prefixLookupDepth = 0;

  // These are used as an optimization for map lookup.
  // Since the first step of all map lookups is the same, do it once and
  // pass it down
  StaticList<const void *, 24> mapDataLookups;

  void AddMapDataLookup(MapDataLookup mapDataLookup,
                        const uint8_t *baseAddress);

  void SortResults();

  StaticList<StenoReverseDictionaryResult, 24> results;
  StaticList<StenoStroke, 64> strokes;

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

  // GetDictionaryForOutline is used to determine which dictionary, if any,
  // can provide a definition for the specified outline.
  //
  // One use case is to determine if a higher priority dictionary would
  // override a lookup. To shortcut some processing, dictionaryHint can be
  // provided that is known to have a definition for the specified outline.
  inline const StenoDictionary *GetDictionaryForOutline(
      const StenoStroke *strokes, size_t length,
      const StenoDictionary *dictionaryHint = nullptr) const {
    return GetDictionaryForOutline(
        StenoDictionaryLookup(strokes, length, dictionaryHint));
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

#if ENABLE_DICTIONARY_STATS
  struct Stats {
    size_t lookupCount;
    size_t reverseLookupCount;
    size_t dictionaryForOutlineCount;

    void Reset() {
      lookupCount = 0;
      reverseLookupCount = 0;
      dictionaryForOutlineCount = 0;
    }
  };

  static Stats stats;

  static void ResetStats() { stats.Reset(); }
  static size_t GetLookupCount() { return stats.lookupCount; }
  static size_t GetReverseLookupCount() { return stats.reverseLookupCount; }
  static size_t GetDictionaryForOutlineCount() {
    return stats.dictionaryForOutlineCount;
  }
#endif

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

//---------------------------------------------------------------------------

#pragma once
#include "../container/static_list.h"
#include "../crc32.h"
#include "../malloc_allocate.h"
#include "../str.h"
#include "../stroke.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

#define ENABLE_DICTIONARY_STATS 0
#define ENABLE_DICTIONARY_LOOKUP_CACHE 1
#define ENABLE_DICTIONARY_LOOKUP_CACHE_STATS 0

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

  bool IsStatic() const { return IsStatic(text); }

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

  bool IsStatic() const { return destroyMethod == &Nop; }

  const char *GetText() const { return text; }
  void Destroy() { (*destroyMethod)(this); }

  static const StenoDictionaryLookupResult NO_OP;

  StenoDictionaryLookupResult Clone() const;

  static StenoDictionaryLookupResult CreateInvalid() {
    StenoDictionaryLookupResult result;
    result.text = nullptr;
    result.destroyMethod = &Nop;
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
#if ENABLE_DICTIONARY_LOOKUP_CACHE
  mutable bool updateCache = false;
#endif
};

//---------------------------------------------------------------------------

struct StenoReverseDictionaryResult {
  size_t length;
  StenoStroke *strokes;
  const StenoDictionary *dictionary;
};

struct MapLookupData {
  MapLookupData() { range.max = nullptr; }

  Interval<const void *> range;
  StaticList<const void *, 24> entries;

  bool IsEmpty() const { return entries.IsEmpty(); }
  void Reset() { entries.Reset(); }

  void Add(MapDataLookup mapDataLookup, const uint8_t *baseAddress);
};

class StenoReverseDictionaryLookup : public JavelinMallocAllocate {
public:
  StenoReverseDictionaryLookup(const char *definition,
                               size_t strokeThreshold = MAX_STROKE_THRESHOLD)
      : ignoreStrokeThreshold(strokeThreshold), definition(definition),
        definitionLength(Str::Length(definition)),
        definitionCrc(Crc32::Hash(definition, definitionLength)) {}

  // Results with stroke count equal to, or above this will not be captured.
  // i.e. Only stroke count less than this will be returned.
  const size_t ignoreStrokeThreshold;

  const char *const definition;
  const size_t definitionLength;
  const uint32_t definitionCrc;

  // Used to prevent recursing prefixes too far.
  size_t prefixLookupDepth = 0;

  // These are used as an optimization for map lookup.
  // Since the first step of all map lookups is the same, do it once and
  // pass it into each map dictionary.
  MapLookupData mapLookupData;

  StaticList<StenoReverseDictionaryResult, 24> results;
  StaticList<StenoStroke, 64> strokes;

  static constexpr size_t MAX_STROKE_THRESHOLD = 31;

  void AddResult(const StenoStroke *strokes, size_t length,
                 const StenoDictionary *dictionary);
  bool HasResult(const StenoStroke *strokes, size_t length) const;
  bool HasResults() const { return results.IsNotEmpty(); }
  void SortResults();

  // Returns false if there are no results.
  bool AreAllResultsFromSameDictionary() const;

  // Returns the minimum number of strokes used in the results.
  //
  // Returns 0 if there are no results.
  size_t GetMinimumStrokeCount() const;
};

//---------------------------------------------------------------------------

class PrintDictionaryContext {
public:
  virtual bool HasName() const = 0;
  virtual const char *GetName() const = 0;

  virtual void Print(const StenoStroke *strokes, size_t length,
                     const char *definition) = 0;

  void Print(const StenoStroke &stroke, const char *definition) {
    Print(&stroke, 1, definition);
  }
};

class ConsolePrintDictionaryContext final : public PrintDictionaryContext {
public:
  ConsolePrintDictionaryContext(const char *name) : name(name) {}

  bool HasName() const { return name != nullptr; }
  const char *GetName() const { return name; }

  void Print(const StenoStroke *strokes, size_t length, const char *definition);

private:
  bool isFirst = true;
  const char *name;
};

class LookupDictionaryContext {
public:
  LookupDictionaryContext(size_t maxCount)
      : maxCount(maxCount == 0 ? size_t(-1) : maxCount) {}

  void Add(const StenoStroke *strokes, size_t length, const char *definition,
           const StenoDictionary *dictionary);

  bool IsDone() const { return count >= maxCount; }

  Interval<const void *> mapLookupDataRange;
  StaticList<const void *, 24> mapLookupData;

protected:
  virtual void PrintDefinition(const char *definition);

private:
  size_t count = 0;
  size_t maxCount;
  const char *lastDefinition = "";
  List<const StenoDictionary *> dictionaries;
};

class ConsoleLookupDictionaryContext : public LookupDictionaryContext {
private:
  using super = LookupDictionaryContext;

public:
  ConsoleLookupDictionaryContext(const char *defaultDefinition)
      : super(0), defaultDefinition(defaultDefinition) {}

  ~ConsoleLookupDictionaryContext();

private:
  const char *defaultDefinition;
  List<char *> definitions;

  size_t FindIndex(const char *x) const;
  virtual void PrintDefinition(const char *definition) final;
};

class PrintPrefixContext : public LookupDictionaryContext {
private:
  using super = LookupDictionaryContext;

public:
  PrintPrefixContext(const char *prefix, size_t maxCount)
      : super(maxCount), prefix(prefix), prefixLength(Str::Length(prefix)) {}

  const char *prefix;
  size_t prefixLength;
  MapLookupData mapLookupData;
};

class PrintPartialOutlineContext : public LookupDictionaryContext {
private:
  using super = LookupDictionaryContext;

public:
  PrintPartialOutlineContext(const StenoStroke *strokes, size_t length,
                             size_t maxCount)
      : super(maxCount), strokes(strokes), length(length) {}

  const StenoStroke *const strokes;
  const size_t length;
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

  virtual void
  GetDictionariesForOutline(List<const StenoDictionary *> &results,
                            const StenoDictionaryLookup &lookup) const;

  inline void GetDictionariesForOutline(List<const StenoDictionary *> &results,
                                        const StenoStroke *strokes,
                                        size_t length) const {
    GetDictionariesForOutline(results, StenoDictionaryLookup(strokes, length));
  }

  virtual void
  PrintEntriesWithPartialOutline(PrintPartialOutlineContext &context) const {}

  virtual void PrintEntriesWithPrefix(PrintPrefixContext &context) const {}

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
  virtual bool Remove(const char *dictionaryName, const StenoStroke *strokes,
                      size_t length) {
    return false;
  }

  virtual void ReverseLookup(StenoReverseDictionaryLookup &lookup) const;

  size_t GetMaximumOutlineLength() const { return maximumOutlineLength; }

  virtual void SetParentRecursively(StenoDictionary *parent) {
    this->parent = parent;
  }

  virtual void OnLookupDataChanged() {
    if (parent) {
      parent->OnLookupDataChanged();
    }
  }

  virtual bool IsInternal() const { return false; }
  virtual const char *GetName() const = 0;

  // Returns the lookup dictionary within a WrappedDictionary.
  //
  // This optimization avoids multiple wrapped chain calls.
  virtual StenoDictionary *GetLookupDictionary();

  virtual void PrintInfo(int depth) const;
  virtual void PrintDictionary(PrintDictionaryContext &context) const {}

  virtual void ListDictionaries() const {}
  virtual bool EnableDictionary(const char *name) { return false; }
  virtual bool DisableDictionary(const char *name) { return false; }
  virtual bool ToggleDictionary(const char *name) { return false; }
  virtual void EnableAllDictionaries() {}
  virtual void DisableAllDictionaries() {}

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
  static constexpr size_t SPACES_COUNT = 16;
  static const char SPACES[];
};

//---------------------------------------------------------------------------

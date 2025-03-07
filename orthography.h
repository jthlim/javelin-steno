//---------------------------------------------------------------------------

#pragma once
#include "container/sized_list.h"
#include "malloc_allocate.h"
#include "pattern.h"
#include "stroke.h"
#include "xip_pointer.h"
#include <stddef.h>

//---------------------------------------------------------------------------

#define USE_ORTHOGRAPHY_CACHE 1

//---------------------------------------------------------------------------

struct StenoOrthographyRule {
  const char *testPattern;
  const char *replacement;
};

struct StenoOrthographyAlias {
  const char *text;
  const char *alias;
};

struct StenoOrthographyAutoSuffix {
  StenoStroke stroke;
  const char *text;
};

struct StenoOrthographyReverseAutoSuffix {
  XipPointer<StenoOrthographyAutoSuffix> autoSuffix;
  StenoStroke suppressMask;
  const char *testPattern;
  const char *replacement;
};

//---------------------------------------------------------------------------

struct StenoOrthography {
  SizedList<StenoOrthographyRule> rules;
  SizedList<StenoOrthographyAlias> aliases;
  StenoStroke autoSuffixMask;
  SizedList<StenoOrthographyAutoSuffix> autoSuffixes;
  SizedList<StenoOrthographyReverseAutoSuffix> reverseAutoSuffixes;

  static const StenoOrthography emptyOrthography;

  void Print() const;
};

//---------------------------------------------------------------------------

class StenoCompiledOrthography {
public:
  explicit StenoCompiledOrthography(const StenoOrthography &orthography);

  char *AddSuffix(const char *word, const char *suffix) const;
  char *AddSuffixToPhrase(const char *phrase, const char *suffix) const;

  void PrintInfo() const;

#if USE_ORTHOGRAPHY_CACHE
  void ResetCache();
#endif

  const StenoOrthography &data;

private:
  const Pattern *patterns;

#if USE_ORTHOGRAPHY_CACHE
  struct CacheEntry : public JavelinMallocAllocate {
    void Set(const char *word, const char *suffix, const char *result);

    bool IsEqual(const char *word, const char *suffix) const;

    const char *GetWord() const { return base; }
    const char *GetSuffix() const { return base + suffixOffset; }
    const char *GetResult() const { return base + resultOffset; }
    char *DupResult() const;

    void Reset();

  private:
    char *base;
    uint8_t suffixOffset;
    uint8_t resultOffset;
    uint8_t resultLength;

  public:
    // The index of the currently used CacheEntry for a block.
    //
    // This should be stored in CacheBlock, but put here for better data
    // packing.
    //
    // Only the first entry in each cache block is used.
    uint8_t blockIndex;
  };

  static void LockCache();
  static void UnlockCache();

  static constexpr size_t CACHE_SIZE = 256;
  static constexpr size_t CACHE_ASSOCIATIVITY = 4;
  static constexpr size_t CACHE_BLOCK_COUNT = CACHE_SIZE / CACHE_ASSOCIATIVITY;

  struct CacheBlock {
    // Stored separately for better data packing.
    // uint8_t index;

    CacheEntry entries[CACHE_ASSOCIATIVITY];

    char *Lookup(const char *word, const char *suffix) const;
    void AddEntry(const char *word, const char *suffix, const char *result);
    void Reset();
  };

  mutable CacheBlock cache[CACHE_BLOCK_COUNT];

#endif

  char *AddSuffixInternal(const char *word, const char *suffix) const;

  class BestCandidate;
  void AddCandidates(BestCandidate &bestCandidate, const char *word,
                     const char *suffix, int defaultScore) const;

  static const Pattern *CreatePatterns(const StenoOrthography &orthography);
};

//---------------------------------------------------------------------------

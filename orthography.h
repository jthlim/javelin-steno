//---------------------------------------------------------------------------

#pragma once
#include "list.h"
#include "malloc_allocate.h"
#include "pattern.h"
#include "stroke.h"
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
  StenoOrthographyAutoSuffix *autoSuffix;
  StenoStroke suppressMask;
  const char *testPattern;
  const char *replacement;
};

//---------------------------------------------------------------------------

struct StenoOrthography {
  size_t ruleCount;
  const StenoOrthographyRule *rules;

  size_t aliasCount;
  const StenoOrthographyAlias *aliases;

  StenoStroke autoSuffixMask;
  size_t autoSuffixCount;
  const StenoOrthographyAutoSuffix *autoSuffixes;

  size_t reverseAutoSuffixCount;
  const StenoOrthographyReverseAutoSuffix *reverseAutoSuffixes;

  static const StenoOrthography emptyOrthography;

  void Print() const;
};

//---------------------------------------------------------------------------

class StenoCompiledOrthography {
public:
  explicit StenoCompiledOrthography(const StenoOrthography &orthography);

  char *AddSuffix(const char *word, const char *suffix) const;

  void PrintInfo() const;

  const StenoOrthography &data;

private:
  struct SuffixEntry;

  const Pattern *patterns;

#if USE_ORTHOGRAPHY_CACHE
  struct CacheEntry : public JavelinMallocAllocate {
    CacheEntry(const char *word, const char *suffix, const char *result);
    ~CacheEntry();

    char *const word;
    char *const suffix;
    char *const result;

    bool IsEqual(const char *word, const char *suffix) const;
  };

  static void LockCache();
  static void UnlockCache();

  static const size_t CACHE_SIZE = 256;
  static const size_t CACHE_ASSOCIATIVITY = 4;
  static const size_t CACHE_BLOCK_COUNT = CACHE_SIZE / CACHE_ASSOCIATIVITY;

  struct CacheBlock {
    uint32_t index;
    CacheEntry *entries[CACHE_ASSOCIATIVITY];

    char *Lookup(const char *word, const char *suffix) const;
    void AddEntry(CacheEntry *entry);
  };

  mutable CacheBlock cache[CACHE_BLOCK_COUNT];

#endif

  char *AddSuffixInternal(const char *word, const char *suffix) const;

  void AddCandidates(List<SuffixEntry> &candidates, const char *word,
                     const char *suffix) const;

  static const Pattern *CreatePatterns(const StenoOrthography &orthography);
};

//---------------------------------------------------------------------------

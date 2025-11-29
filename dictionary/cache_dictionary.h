//---------------------------------------------------------------------------

#pragma once
#include "wrapped_dictionary.h"

//---------------------------------------------------------------------------

class StenoCacheDictionary final : public StenoWrappedDictionary {
private:
  using super = StenoWrappedDictionary;

public:
  StenoCacheDictionary(StenoDictionary *dictionary)
      : StenoWrappedDictionary(dictionary) {}

  virtual StenoDictionaryLookupResult
  Lookup(const StenoDictionaryLookup &lookup) const;
  using StenoDictionary::Lookup;

  virtual const StenoDictionary *
  GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const final;
  using super::GetDictionaryForOutline;

  void AddResult(const StenoDictionaryLookup &lookup,
                 const StenoDictionaryLookupResult result,
                 const StenoDictionary *provider);
  void AddNoResult(const StenoDictionaryLookup &lookup);
  void OnLookupDataChanged() final;

  static void PrintInfo();

  virtual bool IsInternal() const { return true; }
  virtual const char *GetName() const;

private:
  static const size_t CACHE_ASSOCIATIVITY = 4;
  static const size_t CACHE_BLOCKS = 64;
  static const size_t MAXIMUM_STROKE_SIZE_TO_CACHE = 4;

#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
  struct OperationStats {
    size_t lengthLimitExceeded;
    size_t hitEmpty;
    size_t hitDefinition;
    size_t hitDictionary;
    size_t miss;
  };
  struct Stats {
    OperationStats lookup;
    OperationStats getDictionaryForOutline;
  };

  static Stats stats;
#endif

  struct CacheEntry {
    uint32_t hash;
    uint8_t strokeLength;
    uint8_t nextCacheEntryIndex; // Conceptually part of CacheBlock, here for
                                 // better packing.
    StenoStroke strokes[MAXIMUM_STROKE_SIZE_TO_CACHE];

    // The definition of the lookup if it is static.
    //
    // Only static definitions are cached, to ensure that memory allocations
    // are not extended beyond the lifecycle of processing steno input.
    mutable const char *staticDefinition;

    const StenoDictionary *provider;

    void Clear();
    bool IsMatch(const StenoDictionaryLookup &lookup) const;
    void AddResult(const StenoDictionaryLookup &lookup,
                   const StenoDictionaryLookupResult result,
                   const StenoDictionary *provider);
  };

  struct CacheBlock {
    CacheEntry entries[CACHE_ASSOCIATIVITY];

    void Clear();
    const CacheEntry *GetCacheEntry(const StenoDictionaryLookup &lookup) const;
    void AddResult(const StenoDictionaryLookup &lookup,
                   const StenoDictionaryLookupResult result,
                   const StenoDictionary *provider);
  };

  struct Cache {
    CacheBlock blocks[CACHE_BLOCKS];

    void Clear();
    const CacheEntry *GetCacheEntry(const StenoDictionaryLookup &lookup) const;
    void AddResult(const StenoDictionaryLookup &lookup,
                   const StenoDictionaryLookupResult result,
                   const StenoDictionary *provider);
  };

  Cache cache;

  StenoDictionaryLookupResult
  LookupInternal(const StenoDictionaryLookup &lookup) const;
};

//---------------------------------------------------------------------------

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
                 const StenoDictionary *provider);
  void AddNoResult(const StenoDictionaryLookup &lookup);
  void OnLookupDataChanged() final;

  virtual const char *GetName() const;

private:
  static const size_t CACHE_ASSOCIATIVITY = 4;
  static const size_t CACHE_BLOCKS = 64;
  static const size_t MAXIMUM_STROKE_SIZE_TO_CACHE = 5;

  struct CacheEntry {
    uint32_t hash;
    uint8_t strokeLength;
    uint8_t nextCacheEntryIndex; // Conceptually part of CacheBlock, here for
                                 // better packing.
    StenoStroke strokes[MAXIMUM_STROKE_SIZE_TO_CACHE];
    const StenoDictionary *provider;

    void Clear() {
      hash = 0;
      strokeLength = 0;
      provider = nullptr;
    }

    bool IsMatch(const StenoDictionaryLookup &lookup) const;

    void AddResult(const StenoDictionaryLookup &lookup,
                   const StenoDictionary *provider);
  };

  struct CacheBlock {
    CacheEntry entries[CACHE_ASSOCIATIVITY];
    void Clear();

    const StenoDictionary *
    GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

    void AddResult(const StenoDictionaryLookup &lookup,
                   const StenoDictionary *provider);
  };

  struct Cache {
    CacheBlock blocks[CACHE_BLOCKS];
    void Clear();

    const StenoDictionary *
    GetDictionaryForOutline(const StenoDictionaryLookup &lookup) const;

    void AddResult(const StenoDictionaryLookup &lookup,
                   const StenoDictionary *provider);
  };

  Cache cache;
};

//---------------------------------------------------------------------------

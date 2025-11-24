//---------------------------------------------------------------------------

#include "cache_dictionary.h"

//---------------------------------------------------------------------------

#if ENABLE_DICTIONARY_LOOKUP_CACHE

// Controls whether invalid lookups should be cached.
//
// Caching invalid lookups results in under 1% improved hit rate.
// Turning this off avoids extra processing / memory used that could be
// caching useful lookups instead.
#define CACHE_INVALID_LOOKUPS 0

//---------------------------------------------------------------------------

#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
#include "../console.h"
StenoCacheDictionary::Stats StenoCacheDictionary::stats;
#endif

//---------------------------------------------------------------------------

void StenoCacheDictionary::CacheEntry::Clear() {
  hash = 0;
  strokeLength = 0;
}

bool StenoCacheDictionary::CacheEntry::IsMatch(
    const StenoDictionaryLookup &lookup) const {
  return hash == lookup.hash && strokeLength == lookup.length &&
         StenoStroke::Equals(strokes, lookup.strokes, strokeLength);
}

void StenoCacheDictionary::CacheEntry::AddResult(
    const StenoDictionaryLookup &lookup,
    const StenoDictionaryLookupResult result,
    const StenoDictionary *dictionary) {
  provider = dictionary;
  if (result.IsStatic()) {
    staticDefinition = result.GetText();
  } else {
    staticDefinition = nullptr;
  }
  hash = lookup.hash;
  strokeLength = lookup.length;
  lookup.strokes->CopyTo(strokes, lookup.length);
}

//---------------------------------------------------------------------------

void StenoCacheDictionary::CacheBlock::Clear() {
  for (CacheEntry &entry : entries) {
    entry.Clear();
  }
}

const StenoCacheDictionary::CacheEntry *
StenoCacheDictionary::CacheBlock::GetCacheEntry(
    const StenoDictionaryLookup &lookup) const {
  for (const CacheEntry &entry : entries) {
    if (entry.IsMatch(lookup)) {
      return &entry;
    }
  }

  return nullptr;
}

void StenoCacheDictionary::CacheBlock::AddResult(
    const StenoDictionaryLookup &lookup,
    const StenoDictionaryLookupResult result,
    const StenoDictionary *dictionary) {
  CacheEntry &entry =
      entries[entries[0].nextCacheEntryIndex++ % CACHE_ASSOCIATIVITY];
  entry.AddResult(lookup, result, dictionary);
}

//---------------------------------------------------------------------------

void StenoCacheDictionary::Cache::Clear() {
  for (CacheBlock &block : blocks) {
    block.Clear();
  }
}

const StenoCacheDictionary::CacheEntry *
StenoCacheDictionary::Cache::GetCacheEntry(
    const StenoDictionaryLookup &lookup) const {
  const CacheBlock &block = blocks[lookup.hash % CACHE_BLOCKS];
  return block.GetCacheEntry(lookup);
}

void StenoCacheDictionary::Cache::AddResult(
    const StenoDictionaryLookup &lookup,
    const StenoDictionaryLookupResult result,
    const StenoDictionary *dictionary) {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
    return;
  }
  CacheBlock &block = blocks[lookup.hash % CACHE_BLOCKS];
  block.AddResult(lookup, result, dictionary);
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoCacheDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
    stats.lookup.lengthLimitExceeded++;
#endif
    return super::Lookup(lookup);
  }
  // Call Internal method tagged as no-inline to avoid stack manipulations on
  // lookup lengths that are too long.
  return LookupInternal(lookup);
}

[[gnu::noinline]]
StenoDictionaryLookupResult StenoCacheDictionary::LookupInternal(
    const StenoDictionaryLookup &lookup) const {
  const CacheEntry *entry = cache.GetCacheEntry(lookup);
  if (entry == nullptr) {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
    stats.lookup.miss++;
#endif
    lookup.updateCache = true;
    return super::Lookup(lookup);
  }

  const char *definition = entry->staticDefinition;
  if (definition) [[likely]] {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
    stats.lookup.hitDefinition++;
#endif
    return StenoDictionaryLookupResult::CreateStaticString(definition);
  }

  const StenoDictionary *provider = entry->provider;
#if CACHE_INVALID_LOOKUPS
  if (provider == nullptr) {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
    stats.lookup.hitEmpty++;
#endif
    return StenoDictionaryLookupResult::CreateInvalid();
  }
#endif

#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
  stats.lookup.hitDictionary++;
#endif
  StenoDictionaryLookupResult result = provider->Lookup(lookup);
  if (result.IsStatic()) {
    entry->staticDefinition = result.GetText();
  }
  return result;
}

const StenoDictionary *StenoCacheDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
    stats.getDictionaryForOutline.lengthLimitExceeded++;
#endif
    return super::GetDictionaryForOutline(lookup);
  }

  const CacheEntry *entry = cache.GetCacheEntry(lookup);
  if (entry == nullptr) {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
    stats.getDictionaryForOutline.miss++;
#endif
    lookup.updateCache = true;
    return super::GetDictionaryForOutline(lookup);
  }

#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
  stats.getDictionaryForOutline.hitDictionary++;
#endif
  return entry->provider;
}

void StenoCacheDictionary::AddResult(const StenoDictionaryLookup &lookup,
                                     const StenoDictionaryLookupResult result,
                                     const StenoDictionary *provider) {
  cache.AddResult(lookup, result, provider);
}

void StenoCacheDictionary::AddNoResult(const StenoDictionaryLookup &lookup) {
#if CACHE_INVALID_LOOKUPS
  cache.AddResult(lookup, StenoDictionaryLookupResult::CreateInvalid(),
                  nullptr);
#endif
}

void StenoCacheDictionary::OnLookupDataChanged() {
  cache.Clear();
  super::OnLookupDataChanged();
}

const char *StenoCacheDictionary::GetName() const { return "#cache"; }

//---------------------------------------------------------------------------

void StenoCacheDictionary::PrintInfo() {
#if ENABLE_DICTIONARY_LOOKUP_CACHE_STATS
  Console::Printf("Dictionary Cache Stats\n");
  Console::Printf("  lookup: %zu, %zu, %zu, %zu, %zu\n",
                  stats.lookup.lengthLimitExceeded, stats.lookup.miss,
                  stats.lookup.hitDefinition, stats.lookup.hitEmpty,
                  stats.lookup.hitDictionary);
  Console::Printf("  getDictionaryForOutline: %zu, %zu, %zu\n",
                  stats.getDictionaryForOutline.lengthLimitExceeded,
                  stats.getDictionaryForOutline.miss,
                  stats.getDictionaryForOutline.hitDictionary);
#endif
}

//---------------------------------------------------------------------------

#endif // ENABLE_DICTIONARY_LOOKUP_CACHE

//---------------------------------------------------------------------------
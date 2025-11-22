//---------------------------------------------------------------------------

#include "cache_dictionary.h"

//---------------------------------------------------------------------------

#if ENABLE_DICTIONARY_LOOKUP_CACHE

//---------------------------------------------------------------------------

const StenoDictionary *const NO_RESULT_DICTIONARY = (StenoDictionary *)1;

//---------------------------------------------------------------------------

bool StenoCacheDictionary::CacheEntry::IsMatch(
    const StenoDictionaryLookup &lookup) const {
  return hash == lookup.hash && strokeLength == lookup.length &&
         StenoStroke::Equals(strokes, lookup.strokes, strokeLength);
}

void StenoCacheDictionary::CacheEntry::AddResult(
    const StenoDictionaryLookup &lookup, const StenoDictionary *dictionary) {
  hash = lookup.hash;
  strokeLength = lookup.length;
  provider = dictionary;
  lookup.strokes->CopyTo(strokes, lookup.length);
}

const StenoDictionary *
StenoCacheDictionary::CacheBlock::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  for (const CacheEntry &entry : entries) {
    if (entry.IsMatch(lookup)) {
      return entry.provider;
    }
  }

  return nullptr;
}

void StenoCacheDictionary::CacheBlock::AddResult(
    const StenoDictionaryLookup &lookup, const StenoDictionary *dictionary) {
  CacheEntry &entry =
      entries[entries[0].nextCacheEntryIndex++ % CACHE_ASSOCIATIVITY];
  entry.AddResult(lookup, dictionary);
}

void StenoCacheDictionary::CacheBlock::Clear() {
  for (CacheEntry &entry : entries) {
    entry.Clear();
  }
}

const StenoDictionary *StenoCacheDictionary::Cache::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  const CacheBlock &block = blocks[lookup.hash % CACHE_BLOCKS];
  return block.GetDictionaryForOutline(lookup);
}

void StenoCacheDictionary::Cache::AddResult(const StenoDictionaryLookup &lookup,
                                            const StenoDictionary *dictionary) {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
    return;
  }
  CacheBlock &block = blocks[lookup.hash % CACHE_BLOCKS];
  block.AddResult(lookup, dictionary);
}

void StenoCacheDictionary::Cache::Clear() {
  for (CacheBlock &block : blocks) {
    block.Clear();
  }
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoCacheDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
    return super::Lookup(lookup);
  }

  const StenoDictionary *cachedResult = cache.GetDictionaryForOutline(lookup);
  if (cachedResult != nullptr) {
    if (cachedResult == NO_RESULT_DICTIONARY) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
    return cachedResult->Lookup(lookup);
  }

  lookup.updateCache = true;
  return super::Lookup(lookup);
}

const StenoDictionary *StenoCacheDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
    return super::GetDictionaryForOutline(lookup);
  }

  const StenoDictionary *cachedResult = cache.GetDictionaryForOutline(lookup);
  if (cachedResult != nullptr) {
    return cachedResult == NO_RESULT_DICTIONARY ? nullptr : cachedResult;
  }

  lookup.updateCache = true;
  return super::GetDictionaryForOutline(lookup);
}

void StenoCacheDictionary::AddResult(const StenoDictionaryLookup &lookup,
                                     const StenoDictionary *provider) {
  cache.AddResult(lookup, provider);
}

void StenoCacheDictionary::AddNoResult(const StenoDictionaryLookup &lookup) {
  cache.AddResult(lookup, NO_RESULT_DICTIONARY);
}

void StenoCacheDictionary::ClearCache() { cache.Clear(); }

const char *StenoCacheDictionary::GetName() const { return "#cache"; }

//---------------------------------------------------------------------------

#endif // ENABLE_DICTIONARY_LOOKUP_CACHE

//---------------------------------------------------------------------------
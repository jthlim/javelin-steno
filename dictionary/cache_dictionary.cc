//---------------------------------------------------------------------------

#include "cache_dictionary.h"

//---------------------------------------------------------------------------

#if ENABLE_DICTIONARY_LOOKUP_CACHE

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
    return super::Lookup(lookup);
  }

  const CacheEntry *entry = cache.GetCacheEntry(lookup);
  if (entry == nullptr) {
    lookup.updateCache = true;
    return super::Lookup(lookup);
  }

  const StenoDictionary *provider = entry->provider;
  if (provider == nullptr) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  const char *definition = entry->staticDefinition;
  if (definition) {
    return StenoDictionaryLookupResult::CreateStaticString(definition);
  }

  StenoDictionaryLookupResult result = provider->Lookup(lookup);
  if (result.IsStatic()) {
    entry->staticDefinition = result.GetText();
  }
  return result;
}

const StenoDictionary *StenoCacheDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  if (lookup.length > MAXIMUM_STROKE_SIZE_TO_CACHE) {
    return super::GetDictionaryForOutline(lookup);
  }

  const CacheEntry *entry = cache.GetCacheEntry(lookup);
  if (entry != nullptr) {
    return entry->provider;
  }

  lookup.updateCache = true;
  return super::GetDictionaryForOutline(lookup);
}

void StenoCacheDictionary::AddResult(const StenoDictionaryLookup &lookup,
                                     const StenoDictionaryLookupResult result,
                                     const StenoDictionary *provider) {
  cache.AddResult(lookup, result, provider);
}

void StenoCacheDictionary::AddNoResult(const StenoDictionaryLookup &lookup) {
  cache.AddResult(lookup, StenoDictionaryLookupResult::CreateInvalid(),
                  nullptr);
}

void StenoCacheDictionary::OnLookupDataChanged() {
  cache.Clear();
  super::OnLookupDataChanged();
}

const char *StenoCacheDictionary::GetName() const { return "#cache"; }

//---------------------------------------------------------------------------

#endif // ENABLE_DICTIONARY_LOOKUP_CACHE

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "reverse_map_dictionary.h"
#include "../str.h"
#include "map_data_lookup.h"

//---------------------------------------------------------------------------

StenoReverseMapDictionary::StenoReverseMapDictionary(
    StenoDictionary *dictionary, const uint8_t *baseAddress,
    const uint8_t *textBlock, size_t textBlockLength)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
      textBlock(textBlock), textBlockLength(textBlockLength) {
  BuildIndex();
}

void StenoReverseMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  if (result.mapDataLookupCount == 0) {
    AddMapDictionaryData(result);
  }
  dictionary->ReverseLookup(result);
  FilterResult(result);
}

void StenoReverseMapDictionary::AddMapDictionaryData(
    StenoReverseDictionaryLookup &result) const {

  size_t indexLeft = 0;
  size_t indexRight = indexSize;
  while (indexLeft + 1 < indexRight) {
    size_t mid = (indexLeft + indexRight) >> 1;
    int compare = strcmp(result.lookup, (const char *)index[mid]);
    if (compare < 0) {
      indexRight = mid;
    } else {
      indexLeft = mid;
    }
  }

  const uint8_t *left = index[indexLeft];
  const uint8_t *right = index[indexRight];

  while (left < right) {
#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK
    // Optimization when top bit of pointer cannot be set.
    const uint8_t *mid = (const uint8_t *)((size_t(left) + size_t(right)) / 2);
#else
    const uint8_t *mid = left + size_t(right - left) / 2;
#endif

    const uint8_t *wordStart = mid;
    while (wordStart[-1] != 0xff) {
      --wordStart;
    }

    // Inline strcmp because the end of the match is useful.
    const uint8_t *p = wordStart;
    const uint8_t *l = (const uint8_t *)result.lookup;
    int compare;
    int cp;
    for (;;) {
      int cl = *l++;
      cp = *p++;

      compare = cl - cp;
      if (compare != 0 || cp == 0) {
        break;
      }
    }

    if (compare < 0) {
      right = wordStart;
      continue;
    }

    while (cp != 0) {
      cp = *p++;
    }

    if (compare > 0) {
      MapDataLookup lookup(p);
      while (lookup.HasData()) {
        ++lookup;
      }
      left = lookup.GetPointer() + 1;
      continue;
    }

    result.AddMapDataLookup(p, baseAddress);
    return;
  }
}

// This ensures that the results are not conflicting with higher priority
// dictionaries.
void StenoReverseMapDictionary::FilterResult(
    StenoReverseDictionaryLookup &result) const {
  size_t newCount = 0;
  for (size_t i = 0; i < result.resultCount; ++i) {
    const StenoReverseDictionaryResult &r = result.results[i];
    if (dictionary->GetDictionaryForOutline(r.strokes, r.length) ==
        r.lookupProvider) {
      result.results[newCount++] = r;
    }
  }
  result.resultCount = newCount;
}

void StenoReverseMapDictionary::BuildIndex() {
  for (size_t i = 0; i < INDEX_SIZE; ++i) {
    size_t offset = 1 + i * textBlockLength / INDEX_SIZE;
    const uint8_t *word = textBlock + offset;

    while (word[-1] != 0xff) {
      --word;
    }

    if (indexSize > 0 && index[indexSize - 1] == word) {
      continue;
    }

    index[indexSize++] = word;
  }

  const uint8_t *end = textBlock + textBlockLength;
  index[indexSize] = end;
}

const char *StenoReverseMapDictionary::GetName() const {
  return "#internal#reverse_map_dictionary";
}

//---------------------------------------------------------------------------

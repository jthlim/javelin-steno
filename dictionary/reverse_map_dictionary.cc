
//---------------------------------------------------------------------------

#include "reverse_map_dictionary.h"
#include "map_data_lookup.h"

//---------------------------------------------------------------------------

StenoReverseMapDictionary::StenoReverseMapDictionary(
    StenoDictionary *dictionary, const uint8_t *baseAddress,
    const SizedList<uint8_t> &textBlock)
    : StenoWrappedDictionary(dictionary), baseAddress(baseAddress),
      textBlock(textBlock) {
  BuildIndex();
}

void StenoReverseMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  if (lookup.mapDataLookups.IsEmpty()) {
    AddMapDictionaryData(lookup);
  }
  super::ReverseLookup(lookup);
  FilterResult(lookup);
}

const uint8_t *
StenoReverseMapDictionary::FindMapDataLookup(const char *text) const {
  size_t indexLeft = 0;
  size_t indexRight = indexSize;
  while (indexLeft + 1 < indexRight) {
    const size_t mid = (indexLeft + indexRight) >> 1;
    const int compare = strcmp(text, (const char *)index[mid]);
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

    const uint8_t *wordStart = StenoTextBlock::FindPreviousWordStart(mid);

    // Inline strcmp because the end of the match is useful.
    const uint8_t *p = wordStart;
    const uint8_t *l = (const uint8_t *)text;
    int compare;
    for (;;) {
      const int cl = *l++;
      const int cp = *p++;

      if (cl != cp || cp == 0) {
        compare = cl - cp;
        break;
      }
    }

    if (compare < 0) {
      right = wordStart;
      continue;
    }

    if (compare > 0) {
      left = StenoTextBlock::FindNextWordStart(p);
      continue;
    }

    return p;
  }
  return nullptr;
}

void StenoReverseMapDictionary::AddMapDictionaryData(
    StenoReverseDictionaryLookup &lookup) const {
  const uint8_t *p = FindMapDataLookup(lookup.definition);

  if (p) {
    lookup.AddMapDataLookup(p, baseAddress);
  }
}

// This ensures that the results are not conflicting with higher priority
// dictionaries.
void StenoReverseMapDictionary::FilterResult(
    StenoReverseDictionaryLookup &lookup) const {
  size_t newCount = 0;
  for (const StenoReverseDictionaryResult &r : lookup.results) {
    if (super::GetDictionaryForOutline(r.strokes, r.length, r.dictionary) ==
        r.dictionary) {
      lookup.results[newCount++] = r;
    }
  }
  lookup.results.SetCount(newCount);
}

void StenoReverseMapDictionary::BuildIndex() {
  for (size_t i = 0; i < INDEX_SIZE; ++i) {
    const size_t offset = 1 + i * textBlock.count / INDEX_SIZE;
    const uint8_t *word =
        StenoTextBlock::FindPreviousWordStart(textBlock.data + offset);

    if (indexSize > 0 && index[indexSize - 1] == word) {
      continue;
    }

    index[indexSize++] = word;
  }

  index[indexSize] = end(textBlock);
}

const char *StenoReverseMapDictionary::GetName() const {
  return "#internal#reverse_map_dictionary";
}

//---------------------------------------------------------------------------

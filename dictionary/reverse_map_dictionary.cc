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

    int compare = strcmp(result.lookup, (const char *)wordStart);
    if (compare < 0) {
      right = wordStart;
    } else if (compare > 0) {
      while (*mid != 0xff) {
        ++mid;
      }
      left = mid + 1;
    } else {
      const uint8_t *p = wordStart;
      while (*p != 0) {
        ++p;
      }
      ++p;

      MapDataLookup mapDataLookup(p);
      while (mapDataLookup.HasData()) {
        result.AddMapDataLookup(mapDataLookup.GetData(baseAddress));
        if (result.IsMapDataLookupFull()) {
          break;
        }
        ++mapDataLookup;
      }

      return;
    }
  }
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

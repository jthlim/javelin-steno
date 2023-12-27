//---------------------------------------------------------------------------

#include "reverse_map_dictionary.h"
#include "../str.h"
#include "map_data_lookup.h"

//---------------------------------------------------------------------------

void StenoReverseMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  if (result.mapDataLookupCount == 0) {
    AddMapDictionaryData(result);
  }
  dictionary->ReverseLookup(result);
}

void StenoReverseMapDictionary::AddMapDictionaryData(
    StenoReverseDictionaryLookup &result) const {
  const uint8_t *left = textBlock + 1;
  const uint8_t *right = textBlock + textBlockLength;

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
      while (*wordStart != 0xff) {
        ++wordStart;
      }
      left = wordStart + 1;
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

const char *StenoReverseMapDictionary::GetName() const {
  return "#internal#reverse_map_dictionary";
}

//---------------------------------------------------------------------------

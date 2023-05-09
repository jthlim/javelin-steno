//---------------------------------------------------------------------------

#include "reverse_map_dictionary.h"
#include "../str.h"

//---------------------------------------------------------------------------

void StenoReverseMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  StenoReverseDictionaryLookup value(result.strokeThreshold, result.lookup);

  dictionary->ReverseLookup(value);
  AddMapDictionaryResults(value);
  AddValidLookupProviders(result, value);
}

void StenoReverseMapDictionary::AddMapDictionaryResults(
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
      while (*p != 0xff) {
        uint32_t offset = p[0] + (p[1] << 7) + (p[2] << 14) + (p[3] << 21);
        StenoReverseMapDictionaryLookup lookup(baseAddress + offset);
        if (ReverseMapDictionaryLookup(lookup)) {
          result.AddResult(lookup.strokes, lookup.length, lookup.provider);
        }
        p += 4;
      }

      return;
    }
  }
}

void StenoReverseMapDictionary::AddValidLookupProviders(
    StenoReverseDictionaryLookup &result,
    StenoReverseDictionaryLookup &value) const {
  for (size_t i = 0; i < value.resultCount; ++i) {
    const StenoReverseDictionaryResult &v = value.results[i];
    if (dictionary->GetLookupProvider(v.strokes, v.length) ==
        v.lookupProvider) {
      result.AddResult(v.strokes, v.length, v.lookupProvider);
    }
  }
}

const char *StenoReverseMapDictionary::GetName() const {
  return "#internal#reverse_map_dictionary";
}

//---------------------------------------------------------------------------

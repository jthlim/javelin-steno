
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
  if (lookup.mapLookupData.IsEmpty()) {
    AddMapDictionaryData(lookup);
  }
  super::ReverseLookup(lookup);
  FilterResult(lookup);
}

void StenoReverseMapDictionary::PrintEntriesWithPrefix(
    PrintPrefixContext &context) const {
  const uint8_t *prefix = FindPrefixLookup(context.prefix);
  while (Str::HasPrefix((const char *)prefix, context.prefix)) {
    const uint8_t *data = StenoTextBlock::FindDataStart(prefix);
    context.mapLookupData.Reset();
    context.mapLookupData.Add(data, baseAddress);
    super::PrintEntriesWithPrefix(context);
    if (context.IsDone()) {
      return;
    }
    prefix = StenoTextBlock::FindNextWordStart(data);
  }
}

const uint8_t *
StenoReverseMapDictionary::FindMapDataLookup(const char *t) const {
  const uint8_t *text = (const uint8_t *)t;
  const uint8_t *left = index[text[0]];
  const uint8_t *right = index[text[0] + 1];

  // Skip first letter as index ensures they're in range.
  ++text;

  while (left < right) {
#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK
    // Optimization when top bit of pointer cannot be set.
    const uint8_t *mid = (const uint8_t *)((size_t(left) + size_t(right)) / 2);
#else
    const uint8_t *mid = left + size_t(right - left) / 2;
#endif

    const uint8_t *wordStart = StenoTextBlock::FindPreviousWordStart(mid);

    const uint8_t *p = wordStart + 1;
    const uint8_t *l = text;

    for (;;) {
      const int cl = *l++;
      const int cp = *p++;

      if (cl > cp) {
        left = StenoTextBlock::FindNextWordStart(p);
        break;
      } else if (cl < cp) {
        right = wordStart;
        break;
      } else if (cp == 0) [[unlikely]] {
        return p;
      }
    }
  }
  return nullptr;
}

const uint8_t *
StenoReverseMapDictionary::FindPrefixLookup(const char *t) const {
  const uint8_t *text = (const uint8_t *)t;
  const uint8_t *left = index[text[0]];
  const uint8_t *right = index[text[0] + 1];

  while (left < right) {
    const uint8_t *mid = left + size_t(right - left) / 2;
    const uint8_t *wordStart = StenoTextBlock::FindPreviousWordStart(mid);

    const int compare =
        Str::Compare((const char *)text, (const char *)wordStart);
    if (compare < 0) {
      right = wordStart;
    } else {
      left = StenoTextBlock::FindNextWordStart(mid);
    }
  }
  return left;
}

const uint8_t *
StenoReverseMapDictionary::FindFirstEntryWithPrefix(int c) const {
  const uint8_t *left = begin(textBlock) + 1;
  const uint8_t *right = end(textBlock) - 1;

  while (left < right) {
    const uint8_t *mid = left + size_t(right - left) / 2;
    const uint8_t *wordStart = StenoTextBlock::FindPreviousWordStart(mid);

    const int firstLetter = *wordStart;
    if (firstLetter >= c) {
      right = wordStart;
    } else {
      left = StenoTextBlock::FindNextWordStart(mid);
    }
  }
  return left;
}

void StenoReverseMapDictionary::AddMapDictionaryData(
    StenoReverseDictionaryLookup &lookup) const {
  const uint8_t *p = FindMapDataLookup(lookup.definition);

  if (p) {
    lookup.mapLookupData.Add(p, baseAddress);
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
  for (size_t i = 0; i < 256; ++i) {
    index[i] = FindFirstEntryWithPrefix((uint8_t) i);
  }
  index[256] = end(textBlock) - 1;
}

const char *StenoReverseMapDictionary::GetName() const {
  return "#internal#reverse_map_dictionary";
}

//---------------------------------------------------------------------------

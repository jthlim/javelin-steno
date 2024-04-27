//---------------------------------------------------------------------------

#include "dictionary.h"
#include "map_data_lookup.h"

#include "../console.h"
#include <stddef.h>
#include <string.h>

//---------------------------------------------------------------------------

const char StenoDictionary::SPACES[SPACES_COUNT + 1] = "                ";

//---------------------------------------------------------------------------

#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK

#if JAVELIN_CPU_CORTEX_M0

__attribute__((naked)) void StenoDictionaryLookupResult::Destroy() {
  asm volatile(R"(
    ldr r0, [r0]
    lsr r0, #1
    bcs 1f
    bx  lr
  1:
    ldr r1, =free
    bx r1
  )");
}

#elif JAVELIN_CPU_CORTEX_M4

__attribute__((naked)) void StenoDictionaryLookupResult::Destroy() {
  asm volatile(R"(
    ldr r0, [r0]
    lsrs r0, #1
    bcs 1f
    bx  lr
  1:
    b.w free
  )");
}

#else

void StenoDictionaryLookupResult::Destroy() {
  if (text & 1) {
    free((void *)(text >> 1));
  }
}

#endif

StenoDictionaryLookupResult StenoDictionaryLookupResult::Clone() const {
  if (text & 1) {
    return CreateDynamicString(Str::Dup(GetText()));
  } else {
    return *this;
  }
}

#else
void StenoDictionaryLookupResult::Nop(StenoDictionaryLookupResult *) {}

void StenoDictionaryLookupResult::FreeText(StenoDictionaryLookupResult *p) {
  free((char *)p->text);
}

StenoDictionaryLookupResult StenoDictionaryLookupResult::Clone() const {
  if (text == nullptr || destroyMethod == &Nop) {
    return *this;
  }

  return CreateDynamicString(Str::Dup(GetText()));
}

#endif

void StenoReverseDictionaryLookup::AddResult(
    const StenoStroke *strokes, size_t length,
    const StenoDictionary *dictionary) {
  // Ignore if above or equal to the threshold
  if (length >= strokeThreshold) {
    return;
  }

  if (HasResult(strokes, length)) {
    return;
  }

  // Ignore if it'll overflow.
  if (results.IsFull() || strokesCount + length > STROKE_COUNT) {
    return;
  }

  StenoReverseDictionaryResult &entry = results.Add();
  entry.length = length;
  entry.strokes = this->strokes + strokesCount;
  entry.dictionary = dictionary;

  strokes->CopyTo(this->strokes + strokesCount, length);
  strokesCount += length;
}

size_t StenoReverseDictionaryLookup::GetMinimumStrokeCount() const {
  if (results.IsEmpty()) {
    return 0;
  }
  size_t minimumLength = results[0].length;
  for (const StenoReverseDictionaryResult &result : results) {
    if (result.length < minimumLength) {
      minimumLength = result.length;
    }
  }
  return minimumLength;
}

bool StenoReverseDictionaryLookup::HasResult(const StenoStroke *strokes,
                                             size_t length) const {

  for (const StenoReverseDictionaryResult &result : results) {
    if (result.length == length &&
        StenoStroke::Equals(result.strokes, strokes, length)) {
      return true;
    }
  }

  return false;
}

void StenoReverseDictionaryLookup::AddMapDataLookup(
    MapDataLookup mapDataLookup, const uint8_t *baseAddress) {
  while (mapDataLookup.HasData()) {
    mapDataLookups.Add(mapDataLookup.GetData(baseAddress));
    if (mapDataLookups.IsFull()) {
      return;
    }
    ++mapDataLookup;
  }
}

void StenoReverseDictionaryLookup::SortResults() {
  results.Sort([](const StenoReverseDictionaryResult *pa,
                  const StenoReverseDictionaryResult *pb) -> int {
    if (pa->length != pb->length) {
      return (int)pa->length - (int)pb->length;
    }

    const uint32_t popCountA = StenoStroke::PopCount(pa->strokes, pa->length);
    const uint32_t popCountB = StenoStroke::PopCount(pb->strokes, pb->length);
    if (popCountA != popCountB) {
      return (int)popCountA - (int)popCountB;
    }

    return int(intptr_t(pa) - intptr_t(pb));
  });
}

bool StenoReverseDictionaryLookup::AreAllFromSameDictionary() const {
  if (results.IsEmpty()) {
    return false;
  }

  const StenoDictionary *firstDictionary = results[0].dictionary;

  for (const StenoReverseDictionaryResult *it = begin(results) + 1;
       it != end(results); ++it) {
    if (it->dictionary != firstDictionary) {
      return false;
    }
  }

  return true;
}

//---------------------------------------------------------------------------

__attribute__((noinline)) void
PrintDictionaryContext::Print(const StenoStroke *strokes, size_t length,
                              const char *definition) {
  if (!hasData) {
    hasData = true;
    Console::Printf("\n\t");
  } else {
    Console::Printf(",\n\t");
  }

  Console::Printf("\"%T\": \"%J\"", strokes, length, definition);
}

//---------------------------------------------------------------------------

const StenoDictionary *StenoDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  StenoDictionaryLookupResult lookupResult = Lookup(lookup);
  const bool result = lookupResult.IsValid();
  lookupResult.Destroy();
  return result ? this : nullptr;
}

void StenoDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {}

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------

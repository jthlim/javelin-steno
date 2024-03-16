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
    const StenoStroke *c, size_t length,
    const StenoDictionary *lookupProvider) {
  // Ignore if above or equal to the threshold
  if (length >= strokeThreshold) {
    return;
  }

  if (HasResult(c, length)) {
    return;
  }

  // Ignore if it'll overflow.
  if (results.IsFull() || strokesCount + length > STROKE_COUNT) {
    return;
  }

  StenoReverseDictionaryResult &result = results.Add();
  result.length = length;
  result.strokes = strokes + strokesCount;
  result.lookupProvider = lookupProvider;

  memcpy(&strokes[strokesCount], c, sizeof(StenoStroke) * length);
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

bool StenoReverseDictionaryLookup::HasResult(const StenoStroke *c,
                                             size_t length) const {

  for (const StenoReverseDictionaryResult &result : results) {
    if (result.length == length &&
        StenoStroke::Equals(result.strokes, c, length)) {
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

//---------------------------------------------------------------------------

const StenoDictionary *StenoDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  StenoDictionaryLookupResult lookupResult = Lookup(lookup);
  bool result = lookupResult.IsValid();
  lookupResult.Destroy();
  return result ? this : nullptr;
}

void StenoDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {}

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------

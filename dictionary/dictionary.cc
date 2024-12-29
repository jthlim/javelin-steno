//---------------------------------------------------------------------------

#include "dictionary.h"
#include "map_data_lookup.h"

#include "../console.h"
#include <stddef.h>
#include <string.h>

//---------------------------------------------------------------------------

const char StenoDictionary::SPACES[SPACES_COUNT + 1] = "                ";

#if ENABLE_DICTIONARY_STATS
StenoDictionary::Stats StenoDictionary::stats;
#endif

//---------------------------------------------------------------------------

#if JAVELIN_PLATFORM_NRF5_SDK || JAVELIN_PLATFORM_PICO_SDK

constexpr StenoDictionaryLookupResult
    StenoDictionaryLookupResult::NO_OP("{:=}");

#if JAVELIN_CPU_CORTEX_M0

[[gnu::naked]] void
StenoDictionaryLookupResult::DestroyInternal(const char *text) {
  asm volatile(R"(
    lsl r1, r0, #2
    bmi 1f
    bx  lr
  1:
    ldr r1, =free
    bx r1
  )");
}

#elif JAVELIN_CPU_CORTEX_M4

void StenoDictionaryLookupResult::DestroyInternal(const char *text) {
  if (!IsStatic(text)) {
    free((char *)text);
  }
}

#endif

const char *StenoDictionaryLookupResult::CloneInternal(const char *text) {
  if (IsStatic(text)) {
    return text;
  } else {
    return Str::Dup(text);
  }
}

#else

const StenoDictionaryLookupResult StenoDictionaryLookupResult::NO_OP =
    StenoDictionaryLookupResult::CreateStaticString("{:=}");

void StenoDictionaryLookupResult::Nop(StenoDictionaryLookupResult *) {}

void StenoDictionaryLookupResult::FreeText(StenoDictionaryLookupResult *p) {
  free((char *)p->text);
}

StenoDictionaryLookupResult StenoDictionaryLookupResult::Clone() const {
  if (text == nullptr || destroyMethod == &Nop) {
    return *this;
  }

  return CreateDup(GetText());
}

#endif

void StenoReverseDictionaryLookup::AddResult(
    const StenoStroke *strokes, size_t length,
    const StenoDictionary *dictionary) {
  // Ignore if above or equal to the threshold.
  if (length >= strokeThreshold) {
    return;
  }

  // Ignore if it'll overflow.
  if (results.IsFull() || !this->strokes.CanAddCount(length)) {
    return;
  }

  // Ignore if it's already in the results.
  if (HasResult(strokes, length)) {
    return;
  }

  StenoReverseDictionaryResult &entry = results.Add();
  entry.length = length;
  entry.strokes = end(this->strokes);
  entry.dictionary = dictionary;

  strokes->CopyTo(entry.strokes, length);
  this->strokes.AddCount(length);
}

size_t StenoReverseDictionaryLookup::GetMinimumStrokeCount() const {
  if (results.IsEmpty()) {
    return 0;
  }
  size_t minimumLength = (size_t)-1;
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
  while ((mapDataLookup.HasData())) {
    // This should never happen if the converter limits work.
    if (mapDataLookups.IsFull()) {
      return;
    }

    mapDataLookups.Add(mapDataLookup.GetData(baseAddress));
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

  const StenoDictionary *firstDictionary = results.Front().dictionary;
  for (const StenoReverseDictionaryResult &result : results.Skip(1)) {
    if (result.dictionary != firstDictionary) {
      return false;
    }
  }

  return true;
}

StenoDictionaryLookupResult
StenoDictionaryLookupResult::CreateFromBuffer(BufferWriter &writer) {
  return CreateDynamicString(writer.TerminateStringAndAdoptBuffer());
}

//---------------------------------------------------------------------------

[[gnu::noinline]] void PrintDictionaryContext::Print(const StenoStroke *strokes,
                                                     size_t length,
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

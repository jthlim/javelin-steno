//---------------------------------------------------------------------------

#include "dictionary.h"
#include "map_data_lookup.h"

#include "../console.h"
#include <stddef.h>
#include <string.h>

//---------------------------------------------------------------------------

constexpr char StenoDictionary::SPACES[SPACES_COUNT + 1] = "                ";

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

#else

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
  if (destroyMethod == &Nop) {
    return *this;
  }

  return CreateDup(GetText());
}

#endif

void StenoReverseDictionaryLookup::AddResult(
    const StenoStroke *strokes, size_t length,
    const StenoDictionary *dictionary) {
  // Ignore if above or equal to the threshold.
  if (length >= ignoreStrokeThreshold) {
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

void MapLookupData::Add(MapDataLookup mapDataLookup,
                        const uint8_t *baseAddress) {
  while ((mapDataLookup.HasData())) {
    // This should never happen if the converter limits work.
    if (entries.IsFull()) {
      break;
    }

    entries.Add(mapDataLookup.GetData(baseAddress));
    ++mapDataLookup;
  }
  if (!entries.IsEmpty()) {
    range.Set(entries.Front(), entries.Back());
  } else {
    range.max = nullptr;
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

bool StenoReverseDictionaryLookup::AreAllResultsFromSameDictionary() const {
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

[[gnu::noinline]] void
ConsolePrintDictionaryContext::Print(const StenoStroke *strokes, size_t length,
                                     const char *definition) {
  const char *format = ",\n\t\"%T\": \"%J\"";
  Console::Printf(format + isFirst, strokes, length, definition);
  isFirst = false;
}

//---------------------------------------------------------------------------

void LookupDictionaryContext::Add(const StenoStroke *strokes, size_t length,
                                  const char *definition,
                                  const StenoDictionary *dictionary) {
  if (IsDone()) {
    return;
  }

  const char *format = ",{\"o\":\"%T\"";
  if (!count) {
    ++format;
  }
  ++count;
  Console::Printf(format, strokes, length);
  if (definition != nullptr) {
    Console::Printf(",\"t\":\"%J\"", definition);
  }

  const char *dictionaryName = dictionary->GetName();
  if (*dictionaryName != '#') {
    const size_t index = dictionaries.FindIndex(dictionary);
    if (index == -1) {
      dictionaries.Add(dictionary);
      Console::Printf(",\"d\":\"%J\"", dictionaryName);
      if (dictionary->CanRemove()) {
        Console::Printf(",\"r\":1");
      }
    } else {
      // If the dictionary has already been used before, emit an index.
      Console::Printf(",\"d\":%zu", index);
    }
  }
  Console::Printf("}");
}

//---------------------------------------------------------------------------

const StenoDictionary *StenoDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  StenoDictionaryLookupResult lookupResult = Lookup(lookup);
  const bool result = lookupResult.IsValid();
  lookupResult.Destroy();
  return result ? this : nullptr;
}

void StenoDictionary::GetDictionariesForOutline(
    List<const StenoDictionary *> &results,
    const StenoDictionaryLookup &lookup) const {
  const StenoDictionary *dictionary = GetDictionaryForOutline(lookup);
  if (dictionary) {
    results.Add(dictionary);
  }
}

void StenoDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {}

StenoDictionary *StenoDictionary::GetLookupDictionary() { return this; }

void StenoDictionary::PrintInfo(int depth) const {
  Console::Printf("%s%s\n", Spaces(depth), GetName());
}

//---------------------------------------------------------------------------

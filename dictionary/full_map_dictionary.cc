//---------------------------------------------------------------------------

#include "full_map_dictionary.h"
#include "../bit.h"
#include "../console.h"
#include "../flash.h"
#include "../mem.h"
#include "dictionary_definition.h"

//---------------------------------------------------------------------------

#define PRINT_HASH_STATS 0

//---------------------------------------------------------------------------

inline bool StenoFullHashMapEntryBlock::IsBitSet(size_t bitIndex) const {
  return (mask & (1 << bitIndex)) != 0;
}

size_t StenoFullHashMapEntryBlock::PopCount() const {
  return Bit<sizeof(uint32_t)>::PopCount(mask);
}

//---------------------------------------------------------------------------

void StenoFullMapDictionary::HashStats::Update(size_t runLength) {
  entries += runLength;
  totalProbes += runLength * (runLength + 1) / 2;
  if (runLength > maxProbe) {
    maxProbe = runLength;
  }
}

void StenoFullMapDictionary::HashStats::Update(const HashStats &stats) {
  entries += stats.entries;
  totalProbes += stats.totalProbes;
  if (stats.maxProbe > maxProbe) {
    maxProbe = stats.maxProbe;
  }
}

//---------------------------------------------------------------------------

struct FullStenoMapDictionaryDataEntry {
  uint32_t textOffset;
  StenoStroke strokes[0];

  bool IsDeleted() const { return strokes[0].IsEmpty(); }

  bool Equals(const StenoStroke *strokes, size_t length) const {
    return StenoStroke::Equals(this->strokes, strokes, length);
  }
};

//---------------------------------------------------------------------------

size_t StenoFullMapDictionaryStrokesDefinition::GetOffset(size_t index) const {
  const size_t blockIndex = index / 32;
  const size_t bitIndex = index % 32;

  const StenoFullHashMapEntryBlock &block = offsets[blockIndex];

  // Take advantage of sign bit to test presence.
  const uint32_t mask = block.mask << (31 - bitIndex);
  if ((mask & 0x80000000) == 0) {
    return (size_t)-1;
  }

  // The dictionary builder decrements baseOffset to account for
  // the current mask bit being 1
  const size_t result =
      Bit<sizeof(uint32_t)>::PopCount(mask) + block.baseOffset;

  // This check saves comparison/branch instructions after inlining.
  if (result == (size_t)-1) {
    __builtin_unreachable();
  }

  return result;
}

bool StenoFullMapDictionaryStrokesDefinition::HasEntry(size_t index) const {
  const size_t blockIndex = index / 32;
  const size_t bitIndex = index % 32;

  return offsets[blockIndex].IsBitSet(bitIndex);
}

size_t StenoFullMapDictionaryStrokesDefinition::GetEntryCount() const {
  size_t entryCount = 0;
  const size_t end = (hashMapMask + 1) / 32;
  for (size_t i = 0; i < end; ++i) {
    entryCount += offsets[i].PopCount();
  }
  return entryCount;
}

void StenoFullMapDictionaryStrokesDefinition::PrintDictionary(
    PrintDictionaryContext &context, size_t strokeLength,
    const uint8_t *textBlock) const {
  if (hashMapMask == 0) [[unlikely]] {
    return;
  }
  const size_t dataStride = 4 * (1 + strokeLength);
  for (const uint8_t *data = this->data; data < (const uint8_t *)offsets;
       data += dataStride) {
    const FullStenoMapDictionaryDataEntry &entry =
        *(const FullStenoMapDictionaryDataEntry *)data;

    if (!entry.IsDeleted()) {
      context.Print(entry.strokes, strokeLength,
                    (char *)textBlock + entry.textOffset);
    }
  }
}

static inline void FindNeedle(const StenoStroke *needle, size_t needleLength,
                              const StenoStroke *&haystack,
                              const StenoStroke *haystackEnd) {
  const StenoStroke firstStroke = *needle;
  const StenoStroke *strokeData = haystack;

#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
  uint32_t scratch0, scratch1;
  asm volatile(R"(
      b 1f

      .align 4
    1:
      cmp %0, %3
      bhs 4f

      ldrd %1, %2, [%0], #8
      cmp %1, %6
      it  ne
      cmpne %2, %6
      bne 1b

      cmp %1, %6
      it  eq
      subeq %0, %0, #4
      
      // Check equals
      push {%4, %0, %5}
    2:
      subs %5, #1
      beq 3f
      ldr %1, [%0], #4
      ldr %2, [%4, #4]!
      cmp %1, %2
      beq 2b

      pop {%4, %0, %5}
      b 1b

    3:
      pop {%4, %0, %5}
      subs %0, #4
      b 5f

    4:
      mov %0, #0

    5:
      // Exit
  )"
               : "+r"(strokeData), "+r"(scratch0), "+r"(scratch1)
               : "r"(haystackEnd), "r"(needle), "r"(needleLength),
                 "r"(firstStroke));
  haystack = strokeData;
#else
  while (strokeData < haystackEnd) {
    if (*strokeData != firstStroke) [[likely]] {
    next:
      ++strokeData;
      continue;
    }

    if (!StenoStroke::Equals(strokeData, needle, needleLength)) [[likely]] {
      goto next;
    }
    haystack = strokeData;
    return;
  }
  haystack = nullptr;
#endif
}

void StenoFullMapDictionaryStrokesDefinition::PrintEntriesWithPartialOutline(
    PrintPartialOutlineContext &context, size_t definitionStrokeLength,
    const uint8_t *textBlock, const StenoDictionary *dictionary) const {
  if (hashMapMask == 0) [[unlikely]] {
    return;
  }

  // This routine finds candidate strokes, then ensured they're valid matches.
  // It is about 4x faster than stepping through each entry and checking if
  // there's a partial match.
  const uintptr_t dataStart = uintptr_t(data);
  const uintptr_t dataEnd = uintptr_t(offsets);
  const uintptr_t wordCount = (dataEnd - dataStart) / 4;
  const size_t dataStride = 4 * (1 + definitionStrokeLength);

  const StenoStroke *strokeData = (const StenoStroke *)dataStart + 1;
  const StenoStroke *searchEnd = strokeData + wordCount - context.length;

  for (;;) {
    FindNeedle(context.strokes, context.length, strokeData, searchEnd);
    if (strokeData == nullptr) {
      return;
    }

    // Now figure out if it's actually a valid match.
    const size_t dataIndex = (size_t(strokeData) - dataStart) / dataStride;
    const FullStenoMapDictionaryDataEntry *entry =
        (const FullStenoMapDictionaryDataEntry *)(dataStart +
                                                  dataIndex * dataStride);

    // Ensure the stroke does not overlap the textOffset data.
    if (strokeData < entry->strokes) {
      strokeData++;
      continue;
    }

    // Ensure the stroke data does not overflow the entry.
    const void *nextEntry = (const void *)(uintptr_t(entry) + dataStride);
    if (strokeData + context.length > nextEntry) {
      strokeData++;
      continue;
    }

    if (entry->IsDeleted()) {
      strokeData++;
      continue;
    }

    context.Add(entry->strokes, definitionStrokeLength,
                (char *)textBlock + entry->textOffset, dictionary);
    if (context.IsDone()) [[unlikely]] {
      return;
    }
    strokeData = (const StenoStroke *)nextEntry;
  }
}

//---------------------------------------------------------------------------

StenoFullMapDictionary::StenoFullMapDictionary(
    const StenoFullMapDictionaryDefinition &definition)
    : StenoDictionary(definition.maximumOutlineLength),
      textBlock(definition.textBlock), definition(definition),
      strokes(CreateStrokeCache(this, definition)) {
  dataRange.min = strokes[1].data;
  dataRange.max = strokes[maximumOutlineLength].offsets;
}

void *StenoFullMapDictionary::operator new(
    size_t size, const StenoFullMapDictionaryDefinition &definition) noexcept {
  const size_t cacheSize = sizeof(StenoFullMapDictionaryStrokesDefinition) *
                           definition.maximumOutlineLength;
  return JavelinMallocAllocate::operator new(size + cacheSize);
}

const FullStenoMapDictionaryDataEntry *
StenoFullMapDictionary::FindEntry(const StenoDictionaryLookup &lookup) const {
  const StenoFullMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapMask == 0) [[unlikely]] {
    return nullptr;
  }

  size_t entryIndex = lookup.hash & strokesDefinition.hashMapMask;
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
  if (offset == (size_t)-1) [[likely]] {
    return nullptr;
  }

  // Size of FullStenoMapDictionaryDataEntry for this length.
  const size_t entrySize = 4 + 4 * lookup.length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const FullStenoMapDictionaryDataEntry &entry =
        (const FullStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      return &entry;
    }

    dataIndex += entrySize;
    if (++entryIndex > strokesDefinition.hashMapMask) [[unlikely]] {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasEntry(entryIndex)) {
      return nullptr;
    }
  }
}

StenoDictionaryLookupResult
StenoFullMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const FullStenoMapDictionaryDataEntry *entry = FindEntry(lookup);
  return entry == nullptr ? StenoDictionaryLookupResult::CreateInvalid()
                          : StenoDictionaryLookupResult::CreateStaticString(
                                textBlock + entry->textOffset);
}

const StenoDictionary *StenoFullMapDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  const FullStenoMapDictionaryDataEntry *entry = FindEntry(lookup);
  return entry == nullptr ? nullptr : this;
}

void StenoFullMapDictionary::PrintEntriesWithPartialOutline(
    PrintPartialOutlineContext &context) const {
  for (size_t length = context.length + 1; length <= maximumOutlineLength;
       ++length) {
    const StenoFullMapDictionaryStrokesDefinition &strokesDefinition =
        strokes[length];

    if (strokesDefinition.hashMapMask == 0) {
      continue;
    }

    strokesDefinition.PrintEntriesWithPartialOutline(context, length, textBlock,
                                                     this);
    if (context.IsDone()) {
      return;
    }
  }
}

void StenoFullMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  if (!dataRange.HasIntersection(lookup.mapLookupData.range)) [[likely]] {
    return;
  }

  size_t strokeLength = 1;
  for (const void *data : lookup.mapLookupData.entries) {
    if (data < dataRange.min) {
      continue;
    }
    if (data >= dataRange.max) {
      return;
    }
    const FullStenoMapDictionaryDataEntry *entry =
        (const FullStenoMapDictionaryDataEntry *)data;
    // Check for deletion
    if (entry->IsDeleted()) {
      continue;
    }

    while (strokes[strokeLength].IsEntryAfter(entry)) {
      ++strokeLength;
    }

    lookup.AddResult(entry->strokes, strokeLength, this);
  }
}

void StenoFullMapDictionary::PrintEntriesWithPrefix(
    PrintPrefixContext &context) const {
  if (!dataRange.HasIntersection(context.mapLookupData.range)) {
    return;
  }

  size_t strokeLength = 1;
  for (const void *data : context.mapLookupData.entries) {
    if (data < dataRange.min) {
      continue;
    }
    if (data >= dataRange.max) {
      return;
    }

    const FullStenoMapDictionaryDataEntry *entry =
        (const FullStenoMapDictionaryDataEntry *)data;
    // Check for deletion
    if (entry->IsDeleted()) {
      continue;
    }

    while (strokes[strokeLength].IsEntryAfter(entry)) {
      ++strokeLength;
    }

    context.Add(entry->strokes, strokeLength,
                (const char *)(textBlock + entry->textOffset), this);
  }
}

bool StenoFullMapDictionary::Remove(const char *name,
                                    const StenoStroke *strokes, size_t length) {
  const StenoFullMapDictionaryStrokesDefinition &strokesDefinition =
      this->strokes[length];

  if (strokesDefinition.hashMapMask == 0) {
    return false;
  }

  const uint32_t hash = StenoStroke::Hash(strokes, length);
  size_t entryIndex = hash & strokesDefinition.hashMapMask;
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
  if (offset == (size_t)-1) {
    return false;
  }

  const size_t entrySize = 4 + 4 * length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const FullStenoMapDictionaryDataEntry &entry =
        (const FullStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(strokes, length)) {
      // Found it! Delete it.
      constexpr StenoStroke emptyStroke(0);
      Flash::Write(&entry.strokes, &emptyStroke, sizeof(StenoStroke),
                   FlashWriteMode::PRESERVE);
      return true;
    }

    dataIndex += entrySize;
    if (++entryIndex > strokesDefinition.hashMapMask) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasEntry(entryIndex)) {
      return false;
    }
  }
  return false;
}

const char *StenoFullMapDictionary::GetName() const { return definition.name; }

void StenoFullMapDictionary::PrintInfo(int depth) const {
  const StenoFullMapDictionaryStrokesDefinition &lastStrokeDefinition =
      strokes[maximumOutlineLength];

  const uint8_t *start = (const uint8_t *)&definition;
  const uint8_t *end =
      (const uint8_t *)(lastStrokeDefinition.offsets +
                        (lastStrokeDefinition.hashMapMask + 1) / 32);

  Console::Printf("%s%s: %zu bytes\n", Spaces(depth), GetName(), end - start);

#if PRINT_HASH_STATS
  HashStats overallStats;
  for (size_t i = 1; i <= maximumOutlineLength; ++i) {
    const StenoFullMapDictionaryStrokesDefinition &strokesDefinition =
        strokes[i];

    HashStats stats;
    size_t runLength = 0;
    for (size_t j = 0; j <= strokesDefinition.hashMapMask; ++j) {
      if (strokesDefinition.HasEntry(j)) {
        ++runLength;
      } else {
        stats.Update(runLength);
        runLength = 0;
      }
    }
    stats.Update(runLength);
    overallStats.Update(stats);

    const int probesPerEntry = stats.totalProbes * 100 / stats.entries;
    Console::Printf("%s%zu strokes: %zu entries, %zu probes total, %d.%02d "
                    "probes/entry, %zu max probes, %zu hash table size\n",
                    Spaces(depth + 2), i, stats.entries, stats.totalProbes,
                    probesPerEntry / 100, probesPerEntry % 100, stats.maxProbe,
                    strokesDefinition.hashMapMask + 1);
  }

  const int probesPerEntry =
      overallStats.totalProbes * 100 / overallStats.entries;
  Console::Printf("%sOverall: %zu entries, %zu probes total, %d.%02d "
                  "probes/entry, %zu max probes\n",
                  Spaces(depth + 2), overallStats.entries,
                  overallStats.totalProbes, probesPerEntry / 100,
                  probesPerEntry % 100, overallStats.maxProbe);
#endif
}

void StenoFullMapDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  for (size_t i = 1; i <= maximumOutlineLength; ++i) {
    strokes[i].PrintDictionary(context, i, textBlock);
  }
}

const StenoFullMapDictionaryStrokesDefinition *
StenoFullMapDictionary::CreateStrokeCache(
    StenoFullMapDictionary *object,
    const StenoFullMapDictionaryDefinition &definition) {
  const size_t byteSize = sizeof(StenoFullMapDictionaryStrokesDefinition) *
                          definition.maximumOutlineLength;
  StenoFullMapDictionaryStrokesDefinition *strokes =
      (StenoFullMapDictionaryStrokesDefinition *)(object + 1);
  Mem::Copy(strokes, definition.strokes, byteSize);
  return strokes - 1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../str.h"
#include "../unit_test.h"
#include "test_dictionary.h"
#include <assert.h>

TEST_BEGIN("MapDictionary: Single stroke lookup test") {
  // spellchecker: disable
  const StenoStroke strokes[1] = {
      StenoStroke("TEFT"),
  };
  // spellchecker: enable

  StenoFullMapDictionary *mainDictionary = new (TestDictionary::fullDefinition)
      StenoFullMapDictionary(TestDictionary::fullDefinition);

  auto lookup = mainDictionary->Lookup(strokes, 1);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "test"));
  lookup.Destroy();
  delete mainDictionary;
}
TEST_END

TEST_BEGIN("MapDictionary: Double stroke lookup test") {
  // spellchecker: disable
  const StenoStroke strokes[2] = {
      StenoStroke("TEFT"),
      StenoStroke("-D"),
  };
  // spellchecker: enable

  StenoFullMapDictionary *mainDictionary = new (TestDictionary::fullDefinition)
      StenoFullMapDictionary(TestDictionary::fullDefinition);

  auto lookup = mainDictionary->Lookup(strokes, 2);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "tested"));
  lookup.Destroy();

  delete mainDictionary;
}
TEST_END

//---------------------------------------------------------------------------

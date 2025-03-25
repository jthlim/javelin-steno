//---------------------------------------------------------------------------

#include "full_map_dictionary.h"
#include "../bit.h"
#include "../console.h"
#include "../flash.h"
#include "../mem.h"

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
  StenoStroke strokes[1];

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
  const size_t entryCount = GetEntryCount();
  for (size_t i = 0; i < entryCount; ++i) {
    const size_t dataIndex = 4 * i * (1 + strokeLength);
    const FullStenoMapDictionaryDataEntry &entry =
        (const FullStenoMapDictionaryDataEntry &)data[dataIndex];

    if (!entry.strokes[0].IsEmpty()) {
      context.Print(entry.strokes, strokeLength,
                    (char *)textBlock + entry.textOffset);
    }
  }
}

//---------------------------------------------------------------------------

StenoFullMapDictionary::StenoFullMapDictionary(
    const StenoFullMapDictionaryDefinition &definition)
    : StenoDictionary(definition.maximumOutlineLength),
      textBlock(definition.textBlock), definition(definition),
      strokes(CreateStrokeCache(definition)) {
  dataRange.min = strokes[1].data;
  dataRange.max = strokes[maximumOutlineLength].offsets;
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

void StenoFullMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  if (lookup.mapLookupData.IsEmpty()) {
    return;
  }

  if (lookup.mapLookupData.Front() >= dataRange.max) {
    return;
  }

  if (lookup.mapLookupData.Back() < dataRange.min) {
    return;
  }

  for (const void *data : lookup.mapLookupData) {
    if (data < dataRange.min) {
      continue;
    }
    if (data >= dataRange.max) {
      return;
    }
    ReverseLookup(lookup, (const FullStenoMapDictionaryDataEntry *)data);
  }
}

void StenoFullMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup,
    const FullStenoMapDictionaryDataEntry *entry) const {
  // Check for deletion
  if (entry->strokes[0].IsEmpty()) {
    return;
  }

  size_t strokeLength = 1;
  while (strokes[strokeLength].IsEntryAfter(entry)) {
    ++strokeLength;
  }

  lookup.AddResult(entry->strokes, strokeLength, this);
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
      Flash::Write(&entry.strokes, &emptyStroke, sizeof(StenoStroke));
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
    const StenoFullMapDictionaryDefinition &definition) {
  const size_t byteSize = sizeof(StenoFullMapDictionaryStrokesDefinition) *
                          definition.maximumOutlineLength;
  StenoFullMapDictionaryStrokesDefinition *strokes =
      (StenoFullMapDictionaryStrokesDefinition *)malloc(byteSize);
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

  const StenoFullMapDictionary mainDictionary(TestDictionary::fullDefinition);

  auto lookup = mainDictionary.Lookup(strokes, 1);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "test"));
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("MapDictionary: Double stroke lookup test") {
  // spellchecker: disable
  const StenoStroke strokes[2] = {
      StenoStroke("TEFT"),
      StenoStroke("-D"),
  };
  // spellchecker: enable

  const StenoFullMapDictionary mainDictionary(TestDictionary::fullDefinition);

  auto lookup = mainDictionary.Lookup(strokes, 2);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "tested"));
  lookup.Destroy();
}
TEST_END

//---------------------------------------------------------------------------

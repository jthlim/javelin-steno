//---------------------------------------------------------------------------

#include "compact_map_dictionary.h"
#include "../bit.h"
#include "../console.h"
#include "../flash.h"
#include "../mem.h"
#include "../str.h"
#include "../uint24.h"
#include "dictionary_definition.h"

//---------------------------------------------------------------------------

inline bool StenoCompactHashMapEntryBlock::IsBitSet(size_t bitIndex) const {
  return (masks[bitIndex / 32] & (1 << (bitIndex & 31))) != 0;
}

size_t StenoCompactHashMapEntryBlock::PopCount() const {
  size_t result = 0;
  for (size_t i = 0; i < sizeof(masks) / sizeof(*masks); ++i) {
    result += Bit<sizeof(uint32_t)>::PopCount(masks[i]);
  }
  return result;
}

//---------------------------------------------------------------------------

struct CompactStenoMapDictionaryDataEntry {
  Uint24 textOffset;
  Uint24 strokes[1];

  bool Equals(const StenoStroke *strokes, size_t length) const;
  void ExpandTo(StenoStroke *strokes, size_t length) const;
};

inline bool
CompactStenoMapDictionaryDataEntry::Equals(const StenoStroke *strokes,
                                           size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    if (strokes[i] != this->strokes[i].ToUint32()) {
      return false;
    }
  }
  return true;
}

void CompactStenoMapDictionaryDataEntry::ExpandTo(StenoStroke *strokes,
                                                  size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    strokes[i] = this->strokes[i].ToUint32();
  }
}

//---------------------------------------------------------------------------

size_t
StenoCompactMapDictionaryStrokesDefinition::GetOffset(size_t index) const {
  const size_t blockIndex = index / 128;
  const size_t blockBitIndex = index % 128;
  const size_t maskIndex = blockBitIndex / 32;
  const size_t bitIndex = blockBitIndex % 32;

  const StenoCompactHashMapEntryBlock &block = offsets[blockIndex];

  // Take advantage of sign bit to test presence.
  const uint32_t mask = block.masks[maskIndex] << (31 - bitIndex);
  if ((mask & 0x80000000) == 0) {
    return (size_t)-1;
  }

  // The dictionary builder decrements baseOffset to account for
  // the current mask bit being 1
  size_t result = Bit<sizeof(uint32_t)>::PopCount(mask) + block.baseOffset;
  for (size_t i = 0; i < maskIndex; ++i) {
    result += Bit<sizeof(uint32_t)>::PopCount(block.masks[i]);
  }

  return result;
}

bool StenoCompactMapDictionaryStrokesDefinition::HasEntry(size_t index) const {
  const size_t blockIndex = index / 128;
  const size_t blockBitIndex = index % 128;

  return offsets[blockIndex].IsBitSet(blockBitIndex);
}

size_t StenoCompactMapDictionaryStrokesDefinition::GetEntryCount() const {
  size_t entryCount = 0;
  const size_t end = (hashMapMask + 1) / 128;
  for (size_t i = 0; i < end; ++i) {
    entryCount += offsets[i].PopCount();
  }
  return entryCount;
}

void StenoCompactMapDictionaryStrokesDefinition::PrintDictionary(
    PrintDictionaryContext &context, size_t strokeLength,
    const uint8_t *textBlock) const {
  const size_t entryCount = GetEntryCount();
  StenoStroke strokes[strokeLength];
  for (size_t i = 0; i < entryCount; ++i) {
    const size_t dataIndex = 3 * i * (1 + strokeLength);
    const CompactStenoMapDictionaryDataEntry &entry =
        (const CompactStenoMapDictionaryDataEntry &)data[dataIndex];

    for (size_t j = 0; j < strokeLength; ++j) {
      strokes[j] = entry.strokes[j].ToUint32();
    }

    if (!strokes[0].IsEmpty()) {
      context.Print(strokes, strokeLength,
                    (char *)textBlock + entry.textOffset.ToUint32());
    }
  }
}

//---------------------------------------------------------------------------

StenoCompactMapDictionary::StenoCompactMapDictionary(
    const StenoCompactMapDictionaryDefinition &definition)
    : StenoDictionary(definition.maximumOutlineLength),
      textBlock(definition.textBlock), definition(definition),
      strokes(CreateStrokeCache(definition)) {
  dataRange.min = strokes[1].data;
  dataRange.max = strokes[maximumOutlineLength].offsets;
}

const CompactStenoMapDictionaryDataEntry *StenoCompactMapDictionary::FindEntry(
    const StenoDictionaryLookup &lookup) const {
  const StenoCompactMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapMask == 0) {
    return nullptr;
  }

  size_t entryIndex = lookup.hash & strokesDefinition.hashMapMask;
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
  if (offset == (size_t)-1) {
    return nullptr;
  }

  // Size of CompactStenoMapDictionaryDataEntry for this length.
  const size_t entrySize = 3 + 3 * lookup.length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const CompactStenoMapDictionaryDataEntry &entry =
        (const CompactStenoMapDictionaryDataEntry &)
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
StenoCompactMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const CompactStenoMapDictionaryDataEntry *entry = FindEntry(lookup);
  return entry == nullptr ? StenoDictionaryLookupResult::CreateInvalid()
                          : StenoDictionaryLookupResult::CreateStaticString(
                                textBlock + entry->textOffset.ToUint32());
}

const StenoDictionary *StenoCompactMapDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {
  const CompactStenoMapDictionaryDataEntry *entry = FindEntry(lookup);
  return entry == nullptr ? nullptr : this;
}

void StenoCompactMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  if (!dataRange.HasIntersection(lookup.mapLookupDataRange)) {
    return;
  }

  for (const void *data : lookup.mapLookupData) {
    if (data < dataRange.min) {
      continue;
    }
    if (data >= dataRange.max) {
      return;
    }
    ReverseLookup(lookup, (const CompactStenoMapDictionaryDataEntry *)data);
  }
}

void StenoCompactMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup,
    const CompactStenoMapDictionaryDataEntry *entry) const {
  size_t strokeLength = 1;
  while (strokes[strokeLength].IsEntryAfter(entry)) {
    ++strokeLength;
  }

  StenoStroke strokes[strokeLength];
  entry->ExpandTo(strokes, strokeLength);

  // Check for deletion
  if (strokes[0].IsEmpty()) {
    return;
  }

  lookup.AddResult(strokes, strokeLength, this);
}

bool StenoCompactMapDictionary::Remove(const char *name,
                                       const StenoStroke *strokes,
                                       size_t length) {
  const StenoCompactMapDictionaryStrokesDefinition &strokesDefinition =
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

  const size_t entrySize = 3 + 3 * length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const CompactStenoMapDictionaryDataEntry &entry =
        (const CompactStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(strokes, length)) {
      const Uint24 emptyStroke = Uint24::Create(0);
      Flash::Write(&entry.strokes, &emptyStroke, sizeof(Uint24),
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

const char *StenoCompactMapDictionary::GetName() const {
  return definition.name;
}

void StenoCompactMapDictionary::PrintInfo(int depth) const {
  const StenoCompactMapDictionaryStrokesDefinition &lastStrokeDefinition =
      strokes[maximumOutlineLength];

  const uint8_t *start = (const uint8_t *)&definition;
  const uint8_t *end =
      (const uint8_t *)(lastStrokeDefinition.offsets +
                        (lastStrokeDefinition.hashMapMask + 1) / 128);

  Console::Printf("%s%s: %zu bytes\n", Spaces(depth), GetName(), end - start);
}

void StenoCompactMapDictionary::PrintDictionary(
    PrintDictionaryContext &context) const {
  for (size_t i = 1; i <= maximumOutlineLength; ++i) {
    strokes[i].PrintDictionary(context, i, textBlock);
  }
}

const StenoCompactMapDictionaryStrokesDefinition *
StenoCompactMapDictionary::CreateStrokeCache(
    const StenoCompactMapDictionaryDefinition &definition) {
  const size_t byteSize = sizeof(StenoCompactMapDictionaryStrokesDefinition) *
                          definition.maximumOutlineLength;
  StenoCompactMapDictionaryStrokesDefinition *strokes =
      (StenoCompactMapDictionaryStrokesDefinition *)malloc(byteSize);
  Mem::Copy(strokes, definition.strokes, byteSize);
  return strokes - 1;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"
#include "test_dictionary.h"
#include <assert.h>

TEST_BEGIN("MapDictionary: Single stroke lookup test") {
  // spellchecker: disable
  const StenoStroke strokes[1] = {
      StenoStroke("TEFT"),
  };
  // spellchecker: enable

  const StenoCompactMapDictionary mainDictionary(TestDictionary::definition);

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

  const StenoCompactMapDictionary mainDictionary(TestDictionary::definition);

  auto lookup = mainDictionary.Lookup(strokes, 2);
  assert(lookup.IsValid());
  assert(Str::Eq(lookup.GetText(), "tested"));
  lookup.Destroy();
}
TEST_END

//---------------------------------------------------------------------------

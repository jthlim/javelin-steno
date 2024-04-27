//---------------------------------------------------------------------------

#include "compact_map_dictionary.h"
#include "../bit.h"
#include "../console.h"
#include "../str.h"
#include "../uint24.h"

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
  uint32_t mask = block.masks[maskIndex];
  mask <<= (31 - bitIndex);
  if ((mask & 0x80000000) == 0) {
    return (size_t)-1;
  }

  // mask << 1 prevents counting the current bit.
  size_t result = Bit<sizeof(uint32_t)>::PopCount(mask << 1) + block.baseOffset;
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
  for (size_t i = 0; i < hashMapSize / 128; ++i) {
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
    context.Print(strokes, strokeLength,
                  (char *)textBlock + entry.textOffset.ToUint32());
  }
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoCompactMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoCompactMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapSize == 0) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t entryIndex = lookup.hash & (strokesDefinition.hashMapSize - 1);
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
  if (offset == (size_t)-1) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  // Size of CompactStenoMapDictionaryDataEntry for this length.
  const size_t entrySize = 3 + 3 * lookup.length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const CompactStenoMapDictionaryDataEntry &entry =
        (const CompactStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      const uint8_t *text = textBlock + entry.textOffset.ToUint32();
      return StenoDictionaryLookupResult::CreateStaticString(text);
    }

    dataIndex += entrySize;
    if (++entryIndex >= strokesDefinition.hashMapSize) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasEntry(entryIndex)) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }
}

const StenoDictionary *StenoCompactMapDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {

  const StenoCompactMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapSize == 0) {
    return nullptr;
  }

  size_t entryIndex = lookup.hash & (strokesDefinition.hashMapSize - 1);
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
  if (offset == (size_t)-1) {
    return nullptr;
  }

  const size_t entrySize = 3 + 3 * lookup.length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const CompactStenoMapDictionaryDataEntry &entry =
        (const CompactStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      return this;
    }

    dataIndex += entrySize;
    if (++entryIndex >= strokesDefinition.hashMapSize) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasEntry(entryIndex)) {
      return nullptr;
    }
  }
}

void StenoCompactMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup) const {
  for (const void *data : lookup.mapDataLookups) {
    ReverseLookup(lookup, data);
  }
}

void StenoCompactMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &lookup, const void *data) const {
  // Quick reject
  if (data < strokes[1].data) {
    return;
  }
  if (data >= strokes[maximumOutlineLength].offsets) {
    return;
  }

  for (size_t i = 1; i <= maximumOutlineLength; ++i) {
    const StenoCompactMapDictionaryStrokesDefinition &strokeDefinition =
        strokes[i];

    if (!strokeDefinition.ContainsData(data)) {
      continue;
    }

    // There is a match! Convert it to StenoStrokes.
    const CompactStenoMapDictionaryDataEntry *entry =
        (const CompactStenoMapDictionaryDataEntry *)data;

    StenoStroke strokes[i];
    const size_t strokeLength = i;
    entry->ExpandTo(strokes, strokeLength);
    lookup.AddResult(strokes, strokeLength, this);
    return;
  }
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
                        lastStrokeDefinition.hashMapSize / 128);

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
  memcpy(strokes, definition.strokes, byteSize);
  return strokes - 1;
}

//---------------------------------------------------------------------------

#include "../unit_test.h"
#include "test_dictionary.h"
#include <assert.h>

static StenoCompactMapDictionary mainDictionary(TestDictionary::definition);

TEST_BEGIN("MapDictionary: Single stroke lookup test") {
  // spellchecker: disable
  const StenoStroke strokes[1] = {
      StenoStroke("TEFT"),
  };
  // spellchecker: enable

  auto lookup = mainDictionary.Lookup(strokes, 1);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "test") == 0);
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

  auto lookup = mainDictionary.Lookup(strokes, 2);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "tested") == 0);
  lookup.Destroy();
}
TEST_END

//---------------------------------------------------------------------------

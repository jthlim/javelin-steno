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
StenoMapDictionaryStrokesDefinition::GetCompactOffset(size_t index) const {
  size_t blockIndex = index / 128;
  size_t blockBitIndex = index % 128;
  size_t maskIndex = blockBitIndex / 32;
  size_t bitIndex = blockBitIndex % 32;

  const StenoCompactHashMapEntryBlock &block = compactOffsets[blockIndex];

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

bool StenoMapDictionaryStrokesDefinition::HasCompactEntry(size_t index) const {
  size_t blockIndex = index / 128;
  size_t blockBitIndex = index % 128;

  return compactOffsets[blockIndex].IsBitSet(blockBitIndex);
}

size_t StenoMapDictionaryStrokesDefinition::GetCompactEntryCount() const {
  size_t entryCount = 0;
  for (size_t i = 0; i < hashMapSize / 128; ++i) {
    entryCount += compactOffsets[i].PopCount();
  }
  return entryCount;
}

bool StenoMapDictionaryStrokesDefinition::PrintCompactDictionary(
    bool hasData, size_t strokeLength, char *buffer,
    const uint8_t *textBlock) const {
  size_t entryCount = GetCompactEntryCount();
  for (size_t i = 0; i < entryCount; ++i) {
    if (!hasData) {
      hasData = true;
      Console::Printf("\n\t");
    } else {
      Console::Printf(",\n\t");
    }

    size_t dataIndex = 3 * i * (1 + strokeLength);
    const CompactStenoMapDictionaryDataEntry &entry =
        (const CompactStenoMapDictionaryDataEntry &)data[dataIndex];

    char *p = buffer;
    *p++ = '\"';
    for (size_t j = 0; j < strokeLength; ++j) {
      if (j != 0) {
        *p++ = '/';
      }
      p = StenoStroke(entry.strokes[j].ToUint32()).ToString(p);
    }
    *p++ = '\"';
    *p++ = ':';
    *p++ = ' ';

    *p++ = '\"';
    p = Str::WriteJson(p, (char *)textBlock + entry.textOffset.ToUint32());
    *p++ = '\"';
    Console::Write(buffer, p - buffer);
  }
  return hasData;
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoCompactMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapSize == 0) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t entryIndex = lookup.hash & (strokesDefinition.hashMapSize - 1);
  const size_t offset = strokesDefinition.GetCompactOffset(entryIndex);
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

    if (!strokesDefinition.HasCompactEntry(entryIndex)) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }
}

const StenoDictionary *StenoCompactMapDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {

  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapSize == 0) {
    return nullptr;
  }

  size_t entryIndex = lookup.hash & (strokesDefinition.hashMapSize - 1);
  const size_t offset = strokesDefinition.GetCompactOffset(entryIndex);
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

    if (!strokesDefinition.HasCompactEntry(entryIndex)) {
      return nullptr;
    }
  }
}

void StenoCompactMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  for (size_t i = 0; i < result.mapDataLookupCount; ++i) {
    ReverseLookup(result, result.mapDataLookup[i]);
  }
}

void StenoCompactMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result, const void *data) const {
  // Quick reject
  if (data < strokes[1].data) {
    return;
  }
  if (data >= strokes[cachedMaximumOutlineLength].offsets) {
    return;
  }

  for (size_t i = 1; i <= cachedMaximumOutlineLength; ++i) {
    const StenoMapDictionaryStrokesDefinition &strokeDefinition = strokes[i];

    if (!strokeDefinition.ContainsData(data)) {
      continue;
    }

    // There is a match! Convert it to StenoStrokes.
    const CompactStenoMapDictionaryDataEntry *entry =
        (const CompactStenoMapDictionaryDataEntry *)data;

    StenoStroke strokes[i];
    size_t strokeLength = i;
    entry->ExpandTo(strokes, strokeLength);
    result.AddResult(strokes, strokeLength, this);
    return;
  }
}

const char *StenoCompactMapDictionary::GetName() const {
  return definition.name;
}

void StenoCompactMapDictionary::PrintInfo(int depth) const {
  const StenoMapDictionaryStrokesDefinition &lastStrokeDefinition =
      strokes[cachedMaximumOutlineLength];

  const uint8_t *start = (const uint8_t *)&definition;
  const uint8_t *end =
      (const uint8_t *)(lastStrokeDefinition.compactOffsets +
                        lastStrokeDefinition.hashMapSize / 128);

  Console::Printf("%s%s: %zu bytes\n", Spaces(depth), GetName(), end - start);
}

bool StenoCompactMapDictionary::PrintDictionary(const char *name,
                                                bool hasData) const {
  char *buffer = (char *)malloc(2048);
  for (size_t i = 1; i <= cachedMaximumOutlineLength; ++i) {
    if (strokes[i].PrintCompactDictionary(hasData, i, buffer, textBlock)) {
      hasData = true;
    }
  }
  free(buffer);
  return hasData;
}

size_t StenoCompactMapDictionary::GetMaximumOutlineLength() const {
  return cachedMaximumOutlineLength;
}

const StenoMapDictionaryStrokesDefinition *
StenoCompactMapDictionary::CreateStrokeCache(
    const StenoDictionaryDefinition &definition) {
  size_t byteSize = sizeof(StenoMapDictionaryStrokesDefinition) *
                    definition.maximumOutlineLength;
  StenoMapDictionaryStrokesDefinition *strokes =
      (StenoMapDictionaryStrokesDefinition *)malloc(byteSize);
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

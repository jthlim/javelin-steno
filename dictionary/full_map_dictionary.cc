//---------------------------------------------------------------------------

#include "full_map_dictionary.h"
#include "../bit.h"
#include "../console.h"
#include "../str.h"
#include "../uint24.h"

//---------------------------------------------------------------------------

inline bool StenoFullHashMapEntryBlock::IsBitSet(size_t bitIndex) const {
  return (mask & (1 << bitIndex)) != 0;
}

size_t StenoFullHashMapEntryBlock::PopCount() const {
  return Bit<sizeof(uint32_t)>::PopCount(mask);
}

//---------------------------------------------------------------------------

struct FullStenoMapDictionaryDataEntry {
  uint32_t textOffset;
  StenoStroke strokes[1];

  bool Equals(const StenoStroke *strokes, size_t length) const;
};

inline bool FullStenoMapDictionaryDataEntry::Equals(const StenoStroke *strokes,
                                                    size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    if (strokes[i] != this->strokes[i]) {
      return false;
    }
  }
  return true;
}

//---------------------------------------------------------------------------

size_t StenoMapDictionaryStrokesDefinition::GetFullOffset(size_t index) const {
  size_t blockIndex = index / 32;
  size_t bitIndex = index % 32;

  const StenoFullHashMapEntryBlock &block = fullOffsets[blockIndex];

  // Take advantage of sign bit to test presence.
  uint32_t mask = block.mask;
  mask <<= (31 - bitIndex);
  if ((mask & 0x80000000) == 0) {
    return (size_t)-1;
  }

  // mask << 1 prevents counting the current bit.
  return Bit<sizeof(uint32_t)>::PopCount(mask << 1) + block.baseOffset;
}

bool StenoMapDictionaryStrokesDefinition::HasFullEntry(size_t index) const {
  size_t blockIndex = index / 32;
  size_t bitIndex = index % 32;

  return fullOffsets[blockIndex].IsBitSet(bitIndex);
}

size_t StenoMapDictionaryStrokesDefinition::GetFullEntryCount() const {
  size_t entryCount = 0;
  for (size_t i = 0; i < hashMapSize / 32; ++i) {
    entryCount += fullOffsets[i].PopCount();
  }
  return entryCount;
}

bool StenoMapDictionaryStrokesDefinition::PrintFullDictionary(
    bool hasData, size_t strokeLength, char *buffer,
    const uint8_t *textBlock) const {
  size_t entryCount = GetFullEntryCount();
  for (size_t i = 0; i < entryCount; ++i) {
    if (!hasData) {
      hasData = true;
      Console::Printf("\n\t");
    } else {
      Console::Printf(",\n\t");
    }

    size_t dataIndex = 4 * i * (1 + strokeLength);
    const FullStenoMapDictionaryDataEntry &entry =
        (const FullStenoMapDictionaryDataEntry &)data[dataIndex];

    char *p = buffer;
    *p++ = '\"';
    for (size_t j = 0; j < strokeLength; ++j) {
      if (j != 0) {
        *p++ = '/';
      }
      p = entry.strokes[j].ToString(p);
    }
    *p++ = '\"';
    *p++ = ':';
    *p++ = ' ';

    *p++ = '\"';
    p = Str::WriteJson(p, (char *)textBlock + entry.textOffset);
    *p++ = '\"';
    Console::Write(buffer, p - buffer);
  }
  return hasData;
}

//---------------------------------------------------------------------------

StenoDictionaryLookupResult
StenoFullMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapSize == 0) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t entryIndex = lookup.hash & (strokesDefinition.hashMapSize - 1);
  const size_t offset = strokesDefinition.GetFullOffset(entryIndex);
  if (offset == (size_t)-1) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  // Size of FullStenoMapDictionaryDataEntry for this length.
  const size_t entrySize = 4 + 4 * lookup.length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const FullStenoMapDictionaryDataEntry &entry =
        (const FullStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      const uint8_t *text = textBlock + entry.textOffset;
      return StenoDictionaryLookupResult::CreateStaticString(text);
    }

    dataIndex += entrySize;
    if (++entryIndex >= strokesDefinition.hashMapSize) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasFullEntry(entryIndex)) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }
}

const StenoDictionary *StenoFullMapDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {

  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapSize == 0) {
    return nullptr;
  }

  size_t entryIndex = lookup.hash & (strokesDefinition.hashMapSize - 1);
  const size_t offset = strokesDefinition.GetFullOffset(entryIndex);
  if (offset == (size_t)-1) {
    return nullptr;
  }

  const size_t entrySize = 4 + 4 * lookup.length;
  size_t dataIndex = offset * entrySize;

  for (;;) {
    const FullStenoMapDictionaryDataEntry &entry =
        (const FullStenoMapDictionaryDataEntry &)
            strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      return this;
    }

    dataIndex += entrySize;
    if (++entryIndex >= strokesDefinition.hashMapSize) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasFullEntry(entryIndex)) {
      return nullptr;
    }
  }
}

void StenoFullMapDictionary::ReverseLookup(
    StenoReverseDictionaryLookup &result) const {
  for (size_t i = 0; i < result.mapDataLookupCount; ++i) {
    ReverseLookup(result, result.mapDataLookup[i]);
  }
}

void StenoFullMapDictionary::ReverseLookup(StenoReverseDictionaryLookup &result,
                                           const void *data) const {
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
    const FullStenoMapDictionaryDataEntry *entry =
        (const FullStenoMapDictionaryDataEntry *)data;
    size_t strokeLength = i;
    result.AddResult(entry->strokes, strokeLength, this);
    return;
  }
}

const char *StenoFullMapDictionary::GetName() const { return definition.name; }

void StenoFullMapDictionary::PrintInfo(int depth) const {
  const StenoMapDictionaryStrokesDefinition &lastStrokeDefinition =
      strokes[cachedMaximumOutlineLength];

  const uint8_t *start = (const uint8_t *)&definition;
  const uint8_t *end = (const uint8_t *)(lastStrokeDefinition.fullOffsets +
                                         lastStrokeDefinition.hashMapSize / 32);

  Console::Printf("%s%s: %zu bytes\n", Spaces(depth), GetName(), end - start);
}

bool StenoFullMapDictionary::PrintDictionary(const char *name,
                                             bool hasData) const {
  char *buffer = (char *)malloc(2048);
  for (size_t i = 1; i <= cachedMaximumOutlineLength; ++i) {
    if (strokes[i].PrintFullDictionary(hasData, i, buffer, textBlock)) {
      hasData = true;
    }
  }
  free(buffer);
  return hasData;
}

size_t StenoFullMapDictionary::GetMaximumOutlineLength() const {
  return cachedMaximumOutlineLength;
}

const StenoMapDictionaryStrokesDefinition *
StenoFullMapDictionary::CreateStrokeCache(
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

static StenoFullMapDictionary mainDictionary(TestDictionary::fullDefinition);

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

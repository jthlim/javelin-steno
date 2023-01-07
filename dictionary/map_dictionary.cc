//---------------------------------------------------------------------------

#include "map_dictionary.h"
#include "../console.h"
#include "../str.h"
#include "../uint24.h"
#include "map_dictionary_definition.h"

//---------------------------------------------------------------------------

size_t StenoHashMapEntryBlock::PopCount() const {
  size_t result = 0;
  for (size_t i = 0; i < sizeof(masks) / sizeof(*masks); ++i) {
    result += __builtin_popcount(masks[i]);
  }
  return result;
}

//---------------------------------------------------------------------------

struct StenoMapDictionaryDataEntry {
  Uint24 textOffset;
  Uint24 strokes[1];

  bool Equals(const StenoStroke *strokes, size_t length) const;
  void ExpandTo(StenoStroke *strokes, size_t length) const;
};

inline bool StenoMapDictionaryDataEntry::Equals(const StenoStroke *strokes,
                                                size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    if (strokes[i] != this->strokes[i].ToUint32()) {
      return false;
    }
  }
  return true;
}

void StenoMapDictionaryDataEntry::ExpandTo(StenoStroke *strokes,
                                           size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    strokes[i] = this->strokes[i].ToUint32();
  }
}

//---------------------------------------------------------------------------

size_t StenoMapDictionaryStrokesDefinition::GetOffset(size_t index) const {
  size_t blockIndex = index / 128;
  size_t blockBitIndex = index % 128;
  size_t maskIndex = blockBitIndex / 32;
  size_t bitIndex = blockBitIndex % 32;

  const StenoHashMapEntryBlock &block = offsets[blockIndex];

  // Take advantage of sign bit to test presence.
  uint32_t mask = block.masks[maskIndex];
  mask <<= (31 - bitIndex);
  if ((mask & 0x80000000) == 0) {
    return (size_t)-1;
  }

  // mask << 1 prevents counting the current bit.
  size_t result = __builtin_popcount(mask << 1) + block.baseOffset;
  for (size_t i = 0; i < maskIndex; ++i) {
    result += __builtin_popcount(block.masks[i]);
  }

  return result;
}

size_t StenoMapDictionaryStrokesDefinition::GetEntryCount() const {
  size_t entryCount = 0;
  for (size_t i = 0; i < hashMapSize / 128; ++i) {
    entryCount += offsets[i].PopCount();
  }
  return entryCount;
}

bool StenoMapDictionaryStrokesDefinition::PrintDictionary(
    bool hasData, size_t strokeLength, char *buffer,
    const uint8_t *textBlock) const {
  size_t entryCount = GetEntryCount();
  for (size_t i = 0; i < entryCount; ++i) {
    if (!hasData) {
      hasData = true;
      Console::Write("\n\t", 2);
    } else {
      Console::Write(",\n\t", 3);
    }

    size_t dataIndex = 3 * i * (1 + strokeLength);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)data[dataIndex];

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
StenoMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      definition.strokes[lookup.length - 1];
  if (strokesDefinition.hashMapSize == 0) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex = entryIndex & (strokesDefinition.hashMapSize - 1);

    size_t offset = strokesDefinition.GetOffset(entryIndex);
    if (offset == (size_t)-1) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }

    size_t dataIndex = 3 * offset * (1 + lookup.length);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      const uint8_t *text = definition.textBlock + entry.textOffset.ToUint32();
      return StenoDictionaryLookupResult::CreateStaticString(text);
    }
    ++entryIndex;
  }
}

const StenoDictionary *StenoMapDictionary::GetLookupProvider(
    const StenoDictionaryLookup &lookup) const {
  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      definition.strokes[lookup.length - 1];
  if (strokesDefinition.hashMapSize == 0) {
    return nullptr;
  }

  size_t entryIndex = lookup.hash;
  for (;;) {
    entryIndex = entryIndex & (strokesDefinition.hashMapSize - 1);

    size_t offset = strokesDefinition.GetOffset(entryIndex);
    if (offset == (size_t)-1) {
      return nullptr;
    }

    size_t dataIndex = 3 * offset * (1 + lookup.length);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.strokes, lookup.length)) {
      return this;
    }
    ++entryIndex;
  }
}

bool StenoMapDictionary::ReverseMapDictionaryLookup(
    StenoReverseMapDictionaryLookup &lookup) const {
  const void *data = lookup.data;

  // Quick reject
  if (data < definition.strokes[0].data) {
    return false;
  }
  if (data >= definition.strokes[definition.maximumStrokeCount - 1].offsets) {
    return false;
  }

  for (size_t i = 0; i < definition.maximumStrokeCount; ++i) {
    const StenoMapDictionaryStrokesDefinition &strokeDefinition =
        definition.strokes[i];

    if (!strokeDefinition.ContainsData(data)) {
      continue;
    }

    // There is a match! Convert it to StenoStrokes.
    const StenoMapDictionaryDataEntry *entry =
        (const StenoMapDictionaryDataEntry *)data;
    size_t strokeLength = i + 1;
    entry->ExpandTo(lookup.strokes, strokeLength);
    lookup.length = strokeLength;
    lookup.provider = this;
    return true;
  }
  return false;
}

const char *StenoMapDictionary::GetName() const { return definition.name; }

void StenoMapDictionary::PrintInfo(int depth) const {
  const uint8_t *start = (const uint8_t *)&definition;

  const StenoMapDictionaryStrokesDefinition &lastStrokeDefinition =
      definition.strokes[definition.maximumStrokeCount - 1];

  const uint8_t *end =
      (const uint8_t *)(lastStrokeDefinition.offsets +
                        lastStrokeDefinition.hashMapSize / 128);

  Console::Printf("%s%s: %zu bytes\n", Spaces(depth), GetName(), end - start);
}

bool StenoMapDictionary::PrintDictionary(bool hasData) const {
  char *buffer = (char *)malloc(2048);
  for (size_t i = 0; i < definition.maximumStrokeCount; ++i) {
    if (definition.strokes[i].PrintDictionary(hasData, i + 1, buffer,
                                              definition.textBlock)) {
      hasData = true;
    }
  }
  free(buffer);
  return hasData;
}

//---------------------------------------------------------------------------

unsigned int StenoMapDictionary::GetMaximumMatchLength() const {
  return definition.maximumStrokeCount;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"
#include "main_dictionary.h"
#include <assert.h>

constexpr StenoMapDictionary mainDictionary(MainDictionary::definition);

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

//---------------------------------------------------------------------------

#include "map_dictionary.h"
#include "../console.h"
#include "../str.h"
#include "../uint24.h"
#include "map_dictionary_definition.h"

//---------------------------------------------------------------------------

static const char MISMATCHED_MAGIC_ERROR[] =
    "Dictionary Format Error. Please recreate a full firmware";

//---------------------------------------------------------------------------

struct StenoMapDictionaryDataEntry {
  Uint24 textOffset;
  Uint24 chords[1];

  bool Equals(const StenoChord *chord, size_t length) const;
};

bool StenoMapDictionaryDataEntry::Equals(const StenoChord *chords,
                                         size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    if (chords[i] != this->chords[i].ToUint32()) {
      return false;
    }
  }
  return true;
}

//---------------------------------------------------------------------------

size_t StenoMapDictionaryStrokesDefinition::GetOffset(size_t index) const {
  size_t blockIndex = index / 32;
  size_t bitIndex = index % 32;

  const StenoHashMapEntryBlock &block = offsets[blockIndex];
  if ((block.mask & (1 << bitIndex)) == 0) {
    return 0;
  }

  return __builtin_popcount(block.mask & ~(0xffffffff << bitIndex)) +
         block.baseOffset;
}

size_t StenoMapDictionaryStrokesDefinition::GetEntryCount() const {
  size_t entryCount = 0;
  for (size_t i = 0; i < hashMapSize / 32; ++i) {
    entryCount += __builtin_popcount(offsets[i].mask);
  }
  return entryCount;
}

bool StenoMapDictionaryStrokesDefinition::PrintDictionary(
    bool hasData, size_t chordLength, char *buffer,
    const uint8_t *textBlock) const {
  size_t entryCount = GetEntryCount();
  for (size_t i = 0; i < entryCount; ++i) {
    if (!hasData) {
      hasData = true;
      Console::Write("\n\t", 2);
    } else {
      Console::Write(",\n\t", 3);
    }

    size_t dataIndex = 3 * i * (1 + chordLength);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)data[dataIndex];

    char *p = buffer;
    *p++ = '\"';
    for (size_t j = 0; j < chordLength; ++j) {
      if (j != 0) {
        *p++ = '/';
      }
      p = StenoChord(entry.chords[j].ToUint32()).ToString(p);
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

StenoDictionaryLookup StenoMapDictionary::Lookup(const StenoChord *chords,
                                                 size_t length) const {
  if (definition.magic != STENO_MAP_DICTIONARY_MAGIC) {
    return StenoDictionaryLookup::CreateStaticString(MISMATCHED_MAGIC_ERROR);
  }

  const StenoMapDictionaryStrokesDefinition &strokesDefinition =
      definition.strokes[length - 1];
  if (strokesDefinition.hashMapSize == 0) {
    return StenoDictionaryLookup::CreateInvalid();
  }

  size_t entryIndex = StenoChord::Hash(chords, length);
  for (;;) {
    entryIndex = entryIndex & (strokesDefinition.hashMapSize - 1);

    size_t offset = strokesDefinition.GetOffset(entryIndex);
    if (offset == 0) {
      return StenoDictionaryLookup::CreateInvalid();
    }

    size_t dataIndex = 3 * (offset - 1) * (1 + length);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)strokesDefinition.data[dataIndex];

    if (entry.Equals(chords, length)) {
      const uint8_t *text = definition.textBlock + entry.textOffset.ToUint32();
      return StenoDictionaryLookup::CreateStaticString(text);
    }
    ++entryIndex;
  }
}

StenoMapDictionary::DictionaryStats
StenoMapDictionary::PrintStrokeInfo() const {
  DictionaryStats result = {0, 0};
  for (size_t i = 0; i < definition.maximumStrokeCount; ++i) {
    const StenoMapDictionaryStrokesDefinition &strokes = definition.strokes[i];
    size_t entryCount = strokes.GetEntryCount();
    result.entryCount += entryCount;
    Console::Printf("    %zu-stroke entries: %zu\n", i + 1, entryCount);

    for (size_t j = 0; j < entryCount; ++j) {
      size_t dataIndex = 3 * j * (2 + i);
      const StenoMapDictionaryDataEntry &entry =
          (const StenoMapDictionaryDataEntry &)strokes.data[dataIndex];
      uint32_t textOffset = entry.textOffset.ToUint32();
      if (textOffset > result.maxTextOffset) {
        result.maxTextOffset = textOffset;
      }
    }
  }

  return result;
}

void StenoMapDictionary::PrintInfo() const {
  Console::Printf("  Dictionary\n");
  DictionaryStats stats = PrintStrokeInfo();
  Console::Printf("    Total entries: %zu\n", stats.entryCount);
  const char *lastTextBlockDefinition =
      (const char *)definition.textBlock + stats.maxTextOffset;
  const char *end =
      lastTextBlockDefinition + strlen(lastTextBlockDefinition) + 1;
  size_t textBlockSize = intptr_t(end) - intptr_t(definition.textBlock);
  Console::Printf("    Text block size: %zu bytes\n", textBlockSize);
  size_t totalSize = intptr_t(end) - intptr_t(&definition);
  Console::Printf("    Dictionary size: %zu bytes\n", totalSize);
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
#include <stdio.h>

constexpr StenoMapDictionary mainDictionary(MainDictionary::definition);

TEST_BEGIN("MapDictionary: Single stroke lookup test") {
  const StenoChord chords[1] = {
      StenoChord("TEFT"),
  };

  auto lookup = mainDictionary.Lookup(chords, 1);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "test") == 0);
  lookup.Destroy();
}
TEST_END

TEST_BEGIN("MapDictionary: Double stroke lookup test") {
  // spellchecker: disable
  const StenoChord chords[2] = {
      StenoChord("TEFT"),
      StenoChord("-D"),
  };
  // spellchecker: enable

  auto lookup = mainDictionary.Lookup(chords, 2);
  assert(lookup.IsValid());
  assert(strcmp(lookup.GetText(), "tested") == 0);
  lookup.Destroy();
}
TEST_END

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "map_dictionary.h"
#include "../console.h"
#include "../str.h"
#include "../uint24.h"
#include "map_dictionary_definition.h"

//---------------------------------------------------------------------------

struct StenoMapDictionaryDataEntry {
  Uint24 textOffset;
  Uint24 chords[1];

  bool Equals(const StenoChord *chord, size_t length) const;
};

inline bool StenoMapDictionaryDataEntry::Equals(const StenoChord *chords,
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
    if (offset == 0) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }

    size_t dataIndex = 3 * (offset - 1) * (1 + lookup.length);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.chords, lookup.length)) {
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
    if (offset == 0) {
      return nullptr;
    }

    size_t dataIndex = 3 * (offset - 1) * (1 + lookup.length);
    const StenoMapDictionaryDataEntry &entry =
        (const StenoMapDictionaryDataEntry &)strokesDefinition.data[dataIndex];

    if (entry.Equals(lookup.chords, lookup.length)) {
      return this;
    }
    ++entryIndex;
  }
}

bool StenoMapDictionary::ReverseMapDictionaryLookup(
    StenoReverseDictionaryLookup &result, const void *data) const {
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

    // There is a match! Convert it to StenoChords.
    const StenoMapDictionaryDataEntry *entry =
        (const StenoMapDictionaryDataEntry *)data;
    size_t chordLength = i + 1;
    StenoChord chords[chordLength];
    for (size_t i = 0; i < chordLength; ++i) {
      chords[i] = entry->chords[i].ToUint32();
    }
    result.AddResult(chords, chordLength, this);
    return true;
  }
  return false;
}

const char *StenoMapDictionary::GetName() const { return definition.name; }

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
  const StenoChord chords[1] = {
      StenoChord("TEFT"),
  };
  // spellchecker: enable

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

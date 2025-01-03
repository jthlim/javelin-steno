//---------------------------------------------------------------------------

#include "full_map_dictionary.h"
#include "../bit.h"
#include "../console.h"
#include "../flash.h"
#include "../mem.h"

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

StenoDictionaryLookupResult
StenoFullMapDictionary::Lookup(const StenoDictionaryLookup &lookup) const {
  const StenoFullMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapMask == 0) {
    return StenoDictionaryLookupResult::CreateInvalid();
  }

  size_t entryIndex = lookup.hash & strokesDefinition.hashMapMask;
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
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
    if (++entryIndex > strokesDefinition.hashMapMask) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasEntry(entryIndex)) {
      return StenoDictionaryLookupResult::CreateInvalid();
    }
  }
}

const StenoDictionary *StenoFullMapDictionary::GetDictionaryForOutline(
    const StenoDictionaryLookup &lookup) const {

  const StenoFullMapDictionaryStrokesDefinition &strokesDefinition =
      strokes[lookup.length];

  if (strokesDefinition.hashMapMask == 0) {
    return nullptr;
  }

  size_t entryIndex = lookup.hash & strokesDefinition.hashMapMask;
  const size_t offset = strokesDefinition.GetOffset(entryIndex);
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
    if (++entryIndex > strokesDefinition.hashMapMask) {
      entryIndex = 0;
      dataIndex = 0;
    }

    if (!strokesDefinition.HasEntry(entryIndex)) {
      return nullptr;
    }
  }
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

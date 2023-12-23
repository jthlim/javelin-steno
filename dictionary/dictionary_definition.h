//---------------------------------------------------------------------------

#pragma once
#include "../list.h"
#include "../stroke.h"
#include <stddef.h>

//---------------------------------------------------------------------------
//
// Data Layout:
//
//  A typical hashmap will use null pointers or a null value for empty
//  entries, but this ends up taking quite a bit of space.
//
//  A smaller hash table could just store all of the data in a contiguous
//  block, and use a single bit in the hash map: 0 = null, 1 = data present.
//
//  However, this would then require scanning the entire set of bits up
//  until the index and counting all 1 bits to determine the offset into
//  the data block.
//
//  The approach used here is to divide the hash table into blocks,
//  Each block will have a bit mask, where 0 = null, 1 = present, but
//  there's a `baseOffset` value precalculated, which is a tally of all 1
//  bits in all previous blocks.
//
//  Thus, the offset into the data table can be determined by just doing
//  a population count on the mask up to the bit index being inspected,
//  and adding it to the baseOffset.
//
//  Overall, this reduces the memory requirements from 512 bytes per 128
//  hashmap entries to just 20 bytes -- a 25x savings.
//
//---------------------------------------------------------------------------

class StenoDictionary;
struct StenoDictionaryListEntry;

//---------------------------------------------------------------------------

struct StenoCompactHashMapEntryBlock {
  uint32_t masks[4];
  uint32_t baseOffset;

  bool IsBitSet(size_t bitIndex) const;
  size_t PopCount() const;
};

struct StenoFullHashMapEntryBlock {
  uint32_t mask;
  uint32_t baseOffset;

  bool IsBitSet(size_t bitIndex) const;
  size_t PopCount() const;
};

struct StenoMapDictionaryStrokesDefinition {
  size_t hashMapSize;

  // Stroke -> text information.
  const uint8_t *data;

  // Hash table information -- either CompactStenoHashMapEntryBlock*
  // or FullStenoHashMapEntryBlock*
  union {
    const void *offsets;
    const StenoCompactHashMapEntryBlock *compactOffsets;
    const StenoFullHashMapEntryBlock *fullOffsets;
  };

  bool ContainsData(const void *p) const { return data <= p && p < offsets; }

  size_t GetCompactOffset(size_t index) const;
  bool HasCompactEntry(size_t index) const;
  size_t GetCompactEntryCount() const;
  bool PrintCompactDictionary(bool hasData, size_t strokeLength,
                              const uint8_t *textBlock) const;

  size_t GetFullOffset(size_t index) const;
  bool HasFullEntry(size_t index) const;
  size_t GetFullEntryCount() const;
  bool PrintFullDictionary(bool hasData, size_t strokeLength,
                           const uint8_t *textBlock) const;
};

//---------------------------------------------------------------------------

enum class StenoDictionaryType : uint8_t {
  // COMPACT_MAP uses an offset for every 128 entries and 24-bit values for
  // strokes and text offsets.
  COMPACT_MAP,

  // FULL_MAP uses an offset for every 32 entries and 32-bit value for strokes
  // and text offsets.
  FULL_MAP,

  JEFF_SHOW_STROKE,
  JEFF_NUMBERS,
  JEFF_PHRASING,
  EMILY_SYMBOLS,
};

struct StenoDictionaryDefinition {
  bool defaultEnabled;
  uint8_t maximumOutlineLength;
  StenoDictionaryType type;
  uint8_t _padding3;

  // These fields are only valid for type being compact map or full map.
  const char *name;
  const uint8_t *textBlock;
  const StenoMapDictionaryStrokesDefinition *strokes;

  StenoDictionary *Create() const;
};

//---------------------------------------------------------------------------

constexpr uint32_t STENO_MAP_DICTIONARY_COLLECTION_MAGIC = 0x3243534a; // 'JSC2'

struct StenoDictionaryCollection {
  uint32_t magic;
  uint16_t dictionaryCount;
  bool hasReverseLookup;
  bool _padding7;
  const uint8_t *textBlock;
  size_t textBlockLength;
  const StenoDictionaryDefinition *const dictionaries[];

  void AddDictionariesToList(List<StenoDictionaryListEntry> &list) const;
};

//---------------------------------------------------------------------------
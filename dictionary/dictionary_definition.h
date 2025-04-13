//---------------------------------------------------------------------------

#pragma once
#include "../container/sized_list.h"
#include "../xip_pointer.h"
#include "orthospelling_data.h"
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

template <typename T> class List;
class PrintDictionaryContext;
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

struct StenoCompactMapDictionaryStrokesDefinition {
  size_t hashMapMask;

  // Stroke -> text information.
  const uint8_t *data;

  // Hash map.
  const StenoCompactHashMapEntryBlock *offsets;

  bool IsEntryAfter(const void *p) const { return p >= offsets; }

  size_t GetOffset(size_t index) const;
  bool HasEntry(size_t index) const;
  size_t GetEntryCount() const;
  void PrintDictionary(PrintDictionaryContext &context, size_t strokeLength,
                       const uint8_t *textBlock) const;
};

struct StenoFullMapDictionaryStrokesDefinition {
  size_t hashMapMask;

  // Stroke -> text information.
  const uint8_t *data;

  // Hash map.
  const StenoFullHashMapEntryBlock *offsets;

  bool IsEntryAfter(const void *p) const { return p >= offsets; }

  size_t GetOffset(size_t index) const;
  bool HasEntry(size_t index) const;
  size_t GetEntryCount() const;
  void PrintDictionary(PrintDictionaryContext &context, size_t strokeLength,
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
  ORTHOSPELLING,
};

struct StenoDictionaryDefinition {
  bool defaultEnabled;
  uint8_t maximumOutlineLength;
  StenoDictionaryType type;
  uint8_t options;

  StenoDictionary *Create() const;
};
static_assert(sizeof(StenoDictionaryDefinition) == 4);

struct StenoCompactMapDictionaryDefinition : public StenoDictionaryDefinition {
  XipPointer<char> name;
  const uint8_t *textBlock;
  const StenoCompactMapDictionaryStrokesDefinition *strokes;
};

struct StenoFullMapDictionaryDefinition : public StenoDictionaryDefinition {
  XipPointer<char> name;
  const uint8_t *textBlock;
  const StenoFullMapDictionaryStrokesDefinition *strokes;
};

struct StenoOrthospellingDictionaryDefinition
    : public StenoDictionaryDefinition {
  OrthospellingData data;
};

//---------------------------------------------------------------------------

constexpr uint32_t STENO_MAP_DICTIONARY_COLLECTION_MAGIC = 0x3443534a; // 'JSC4'

struct StenoDictionaryCollection {
  uint32_t magic;
  uint16_t dictionaryCount;
  bool hasReverseLookup;
  bool _padding7;
  SizedList<uint8_t> textBlock;
  SizedList<const uint8_t *> prefixes;
  SizedList<const uint8_t *> suffixes;
  uint32_t timestamp;
  const XipPointer<StenoDictionaryDefinition> dictionaries[];

  // Returns whether the timestamp at the start of the dictionary matches
  // the timestamp at the end of the text block.
  // This is used to detect partial dictionary uploads.
  bool HasMatchingTimestamp() const;
  const void *GetEndOfData() const;
  void AddDictionariesToList(List<StenoDictionaryListEntry> &list) const;
};

//---------------------------------------------------------------------------
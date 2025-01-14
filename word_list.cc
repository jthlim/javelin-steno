//---------------------------------------------------------------------------

#include "word_list.h"
#include "bit.h"
#include "crc.h"
#include "str.h"

//---------------------------------------------------------------------------

constexpr uint32_t WORD_LIST_MAGIC = 0x304c574a;
constexpr WordListData WordList::EMPTY = {WORD_LIST_MAGIC, 32, 0, 0, {{0, 0}}};
WordList WordList::instance;

//---------------------------------------------------------------------------

void WordList::SetData(const WordListData &data) {
  if (data.magic != WORD_LIST_MAGIC) {
    return;
  }
  hashMapMask = data.hashMapSize - 1;
  blocks = data.blocks;
  entries = (const WordListDataEntry *)(data.blocks + data.hashMapSize / 32);
  textData = (const uint8_t *)(entries + data.wordCount);
}

//---------------------------------------------------------------------------

size_t WordList::GetOffset(size_t index) const {
  const size_t blockIndex = index / 32;
  const size_t bitIndex = index % 32;
  const WordListHashMapBlock &block = blocks[blockIndex];

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

bool WordList::HasEntry(size_t index) const {
  const size_t blockIndex = index / 32;
  const size_t bitIndex = index % 32;
  const WordListHashMapBlock &block = blocks[blockIndex];

  return ((block.mask >> bitIndex) & 1) != 0;
}

// Define this to avoid a call to memcmp in GetWordRank.
inline bool WordList::Equals(const uint8_t *a, const uint8_t *b,
                             size_t length) {
  for (; length; --length) {
    if (*a++ != *b++) {
      return false;
    }
  }
  return true;
}

int WordList::GetWordRank(const uint8_t *word, int defaultRank) const {
  const size_t wordLength = Str::Length(word);

  uint32_t index = Crc32(word, wordLength) & hashMapMask;
  const size_t offset = GetOffset(index);
  if (offset == (size_t)-1) {
    return defaultRank;
  }

  const WordListDataEntry *dataEntry = &entries[offset];
  for (;;) {
    if (dataEntry->wordLength == wordLength &&
        Equals(word, textData + dataEntry->offset, wordLength)) {
      return dataEntry->priority;
    }

    ++dataEntry;
    if (++index > hashMapMask) [[unlikely]] {
      index = 0;
      dataEntry = entries;
    }

    if (!HasEntry(index)) {
      return defaultRank;
    }
  }
}

//---------------------------------------------------------------------------

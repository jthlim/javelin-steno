//---------------------------------------------------------------------------

#include "word_list.h"
#include "bit.h"
#include "crc.h"
#include "str.h"

//---------------------------------------------------------------------------

void WordList::SetData(const WordListData &data) {
  if (data.magic != WORD_LIST_MAGIC) {
    return;
  }
#if RUN_TESTS
  if (data.wordCount == 0) {
    hashMapMask = 31;
    blocks = &EMPTY_BLOCK;
    return;
  }
#endif
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

bool WordList::Equals(const uint8_t *a, const uint8_t *b, size_t length) {
  while (length) {
    if (*a++ != *b++) {
      return false;
    }
    --length;
  }
  return true;
}

int WordList::GetWordRank(const uint8_t *word, int defaultRank) {
  const size_t wordLength = Str::Length(word);
  uint32_t hash = Crc32(word, wordLength);

  for (;; ++hash) {
    const size_t offset = GetOffset(hash & hashMapMask);
    if (offset == (size_t)-1) {
      return defaultRank;
    }

    const WordListDataEntry &dataEntry = entries[offset];
    if (dataEntry.wordLength != wordLength) {
      continue;
    }

    const uint8_t *wordBlock = textData + dataEntry.offset;
    if (!Equals(word, wordBlock, wordLength)) {
      continue;
    }

    return dataEntry.priority;
  }
}

//---------------------------------------------------------------------------

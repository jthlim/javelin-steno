//---------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct WordListDataEntry {
  uint32_t offset : 22;
  uint32_t priority : 4;
  uint32_t wordLength : 6;
};
static_assert(sizeof(WordListDataEntry) == 4,
              "Expect WordListDataEntry to be 4 bytes");

struct WordListHashMapBlock {
  uint32_t baseOffset;
  uint32_t mask;
};

struct WordListData {
  uint32_t magic; // JWL0
  uint32_t hashMapSize;
  uint32_t wordCount;
  uint32_t wordBlockSize;
  WordListHashMapBlock blocks[1];
};

//---------------------------------------------------------------------------

class WordList {
public:
  static int GetWordRank(const char *word, int defaultScore = -1) {
    return instance.GetWordRank((const uint8_t *)word, defaultScore);
  }

  void SetData(const WordListData &data);

  static WordList instance;

  static constexpr int MAX_SCORE = 0xf;
  static const WordListData EMPTY;

private:
  int GetWordRank(const uint8_t *word, int defaultScore = -1) const;
  size_t GetOffset(size_t index) const;
  bool HasEntry(size_t index) const;
  static bool Equals(const uint8_t *a, const uint8_t *b, size_t length);

  uint32_t hashMapMask = 31;
  const WordListHashMapBlock *blocks = EMPTY.blocks;
  const WordListDataEntry *entries = nullptr;
  const uint8_t *textData = nullptr;
};

//---------------------------------------------------------------------------

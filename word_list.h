//---------------------------------------------------------------------------

#include "interval.h"
#include "static_list.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

using WordListData = StaticList<uint8_t>;

//---------------------------------------------------------------------------

class WordList {
public:
  // Returns -1 if not found,
  static int GetWordRank(const uint8_t *word);
  static int GetWordRank(const char *word) {
    return GetWordRank((const uint8_t *)word);
  }

  static void SetData(const WordListData &data) {
    // The firmware builder inserts a dummy score before the first word, so
    // offset by 1.
    instance.data.Set(begin(data) + 1, end(data));
  }

  static WordList instance;

  static const int MAX_SCORE = 0xf;

private:
  WordList() : data{.min = DATA + 1, .max = DATA + 1} {}

  Interval<const uint8_t *> data;

  static const uint8_t DATA[];

  static int Compare(const uint8_t *word, const uint8_t *data);
  static bool IsValueByte(uint8_t b) { return b >= 0xf0; };
  static bool ContainsEmoji(const uint8_t *word);
  static const uint8_t *FindValueByteForward(const uint8_t *p);
  static const uint8_t *FindWordStart(const uint8_t *p);
};

//---------------------------------------------------------------------------

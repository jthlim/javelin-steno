//---------------------------------------------------------------------------

#include "interval.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class WordList {
public:
  // Returns -1 if not found,
  static int GetWordRank(const uint8_t *word);
  static int GetWordRank(const char *word) {
    return GetWordRank((const uint8_t *)word);
  }

  static void SetData(const uint8_t *newData, size_t length) {
    instance.data.Set(newData + 1, newData + length);
  }

  static WordList instance;

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

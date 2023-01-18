//---------------------------------------------------------------------------

#include "word_list.h"

//---------------------------------------------------------------------------

int WordList::GetWordRank(const uint8_t *word) {
  if (ContainsEmoji(word)) {
    return -1;
  }

  const uint8_t *left = instance.data;
  const uint8_t *right = instance.dataEnd;

  while (left < right) {
    const uint8_t *mid = left + (right - left) / 2;

    const uint8_t *wordStart = mid;
    while (!IsValueByte(wordStart[-1])) {
      --wordStart;
    }

    int compare = Compare(word, wordStart);
    if (compare < 0) {
      right = wordStart;
    } else {
      const uint8_t *wordEnd = mid;
      while (!IsValueByte(*wordEnd)) {
        ++wordEnd;
      }

      if (compare > 0) {
        left = wordEnd + 1;
      } else {
        return *wordEnd & 0xf;
      }
    }
  }

  return -1;
}

bool WordList::ContainsEmoji(const uint8_t *word) {
  while (*word) {
    if ((*word & 0xf8) == 0xf0) {
      return true;
    }
    ++word;
  }
  return false;
}

int WordList::Compare(const uint8_t *word, const uint8_t *data) {
  for (;;) {
    if (IsValueByte(*data)) {
      return *word;
    }
    // This case folds into the below case.
    // if (*word == '\0') {
    //   return -1;
    // }
    if (*word != *data) {
      return (int)*word - (int)*data;
    }
    ++word;
    ++data;
  }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "word_list.h"

//---------------------------------------------------------------------------

inline const uint8_t *WordList::FindWordStart(const uint8_t *p) {
  while (!IsValueByte(p[-1])) {
    --p;
  }
  return p;
}

inline const uint8_t *WordList::FindValueByteForward(const uint8_t *p) {
  while (!IsValueByte(*p)) {
    ++p;
  }
  return p;
}

int WordList::GetWordRank(const uint8_t *word) {
  if (ContainsEmoji(word)) {
    return -1;
  }

  const uint8_t *left = instance.data;
  const uint8_t *right = instance.dataEnd;

  while (left < right) {
#if JAVELIN_PLATFORM_PICO_SDK || JAVELIN_PLATFORM_NRF5_SDK
    // Optimization when top bit of pointer cannot be set.
    const uint8_t *mid = (const uint8_t *)((size_t(left) + size_t(right)) / 2);
#else
    const uint8_t *mid = left + size_t(right - left) / 2;
#endif
    const uint8_t *wordStart = FindWordStart(mid);

    const int compare = Compare(word, wordStart);
    if (compare < 0) {
      right = wordStart;
    } else {
      const uint8_t *wordEnd = FindValueByteForward(mid);

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
    if (*word >= 0xf0) {
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

//---------------------------------------------------------------------------

#include "word_list.h"

//---------------------------------------------------------------------------

#if JAVELIN_CPU_CORTEX_M4
static uint32_t uqsub8(uint32_t a, uint32_t b) {
  uint32_t result;
  asm("uqsub8 %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
  return result;
}
#endif

inline const uint8_t *WordList::FindWordStart(const uint8_t *p) {
#if JAVELIN_CPU_CORTEX_M4
  uint32_t mask;
  do {
    p -= 4;
    const uint32_t v = *(const uint32_t *)p;
    mask = uqsub8(v, 0xefefefef);
  } while (mask == 0);
  return (p + 4) - (__builtin_clz(mask) >> 3);
#else
  while (!IsValueByte(p[-1])) {
    --p;
  }
  return p;
#endif
}

inline const uint8_t *WordList::FindValueByteForward(const uint8_t *p) {
#if JAVELIN_CPU_CORTEX_M4
  uint32_t mask;
  do {
    const uint32_t v = *(const uint32_t *)p;
    p += 4;
    mask = uqsub8(v, 0xefefefef);
  } while (mask == 0);
  return (p - 4) + (__builtin_clz(__builtin_bswap32((mask))) >> 3);
#else
  while (!IsValueByte(*p)) {
    ++p;
  }
  return p;
#endif
}

int WordList::GetWordRank(const uint8_t *word) {
  if (ContainsEmoji(word)) {
    return -1;
  }

  const uint8_t *left = instance.data.min;
  const uint8_t *right = instance.data.max;

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

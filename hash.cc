//---------------------------------------------------------------------------

#include "hash.h"

//---------------------------------------------------------------------------

inline uint32_t RotateLeft(uint32_t v, int shift) {
  return (v << shift) | (v >> (32 - shift));
}

//---------------------------------------------------------------------------

[[gnu::noinline]] uint32_t JavelinHash::Hash(const uint32_t *data,
                                             size_t wordCount) {
  uint32_t s0 = prime0;
  uint32_t s1 = prime0 + prime1;

  while (wordCount >= 2) {
    s0 = RotateLeft(s0 + data[0] * prime0, 13) * prime1;
    s1 = RotateLeft(s1 + data[1] * prime0, 13) * prime1;
    data += 2;
    wordCount -= 2;
  }
  if (wordCount) {
    s0 = RotateLeft(s0 + data[0] * prime0, 13) * prime1;
  }
  return s0 ^ s1;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "base64.h"

//---------------------------------------------------------------------------

constexpr int8_t BASE64_DECODE_TABLE[256] = {
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, 62, -2, -2, -2, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, -2, -2, -2, -1, -2, -2, -2, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2,
    -2, -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2,
};

//---------------------------------------------------------------------------

bool Base64::IsValid(const char *p) {
  const uint8_t *v = (const uint8_t *)p;

  while (*v) {
    if (BASE64_DECODE_TABLE[*v++] == -2) {
      return false;
    }
  }

  return true;
}

size_t Base64::Decode(uint8_t *destination, const uint8_t *source) {
  uint8_t *const startDestination = destination;

  int value = 0;
  int bytes = 0;
  int dummyBytes = 0;

  while (*source) {
    const int d = BASE64_DECODE_TABLE[*source++];
    if (d == -2) {
      continue;
    }

    value <<= 6;
    if (d >= 0) {
      value += d;
    } else {
      dummyBytes++;
    }

    if (++bytes == 4) {
      *destination++ = value >> 16;
      if (dummyBytes > 1) {
        goto exit;
      }
      *destination++ = value >> 8;
      if (dummyBytes == 1) {
        goto exit;
      }
      *destination++ = value;
      bytes = 0;
      value = 0;
    }
  }

exit:
  return destination - startDestination;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "utf8_pointer.h"

//---------------------------------------------------------------------------

const uint8_t Utf8Pointer::DECODE_TABLE[] = {
    // 0-127
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //

    // 128-191 = invalid codes
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //

    // 192-223 = 2 byte code
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //

    // 224-239 = 3 byte code
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //

    // 240-247 = 4 byte code
    // 248-255 = invalid code
    4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 //
};

uint32_t Utf8Pointer::BytesForCharacterCode(uint32_t c) {
  if (c < 0x80) {
    return 1;
  } else if (c < 0x800) {
    return 2;
  } else if (c < 0x10000) {
    return 3;
  } else {
    return 4;
  }
}

#if JAVELIN_CPU_CORTEX_M4

inline uint32_t bfi(uint32_t previous, uint32_t v, int lsb, int width) {
  __asm("bfi %0, %1, %2, %3" : "+r"(previous) : "r"(v), "i"(lsb), "i"(width) :);
  return previous;
}

uint32_t Utf8Pointer::Read() const {
  switch (DECODE_TABLE[*p]) {
  case 0:
    return 0;

  case 1:
    return p[0];

  case 2:
    return bfi(p[1], p[0], 6, 5);

  case 3:
    return bfi(bfi(p[2], p[1], 6, 6), p[0], 12, 4);

  case 4:
    return bfi(bfi(bfi(p[3], p[2], 6, 6), p[1], 12, 6), p[0], 18, 3);

  default:
    __builtin_unreachable();
  }
}

#else

uint32_t Utf8Pointer::Read() const {
  switch (DECODE_TABLE[*p]) {
  case 0:
    return 0;

  case 1:
    return p[0];

  case 2:
    return (p[0] << 6) ^ p[1] ^ 0x3080;

  case 3:
    return (p[0] << 12) ^ (p[1] << 6) ^ p[2] ^ 0xe2080;

  case 4:
    return (p[0] << 18) ^ (p[1] << 12) ^ (p[2] << 6) ^ p[3] ^ 0x3c82080;

  default:
    __builtin_unreachable();
  }
}
#endif

//---------------------------------------------------------------------------

size_t Utf8::Length(const char *p) {
  Utf8Pointer utf8p(p);
  size_t length = 0;
  while (!utf8p.IsEndOfString()) {
    ++length;
    ++utf8p;
  }
  return length;
}

//---------------------------------------------------------------------------

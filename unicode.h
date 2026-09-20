//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class Unicode {
public:
  static uint32_t ToUpper(uint32_t c);
  static uint32_t ToLower(uint32_t c);

  static bool IsAsciiDigit(uint32_t c) { return '0' <= c && c <= '9'; }
  static bool IsBinaryDigit(uint32_t c) { return '0' <= c && c <= '1'; }
  static bool IsLetter(uint32_t c);
  static bool IsUpper(uint32_t c) { return ToLower(c) != c; }

  static int GetHexValue(uint32_t c);
  static int GetBinaryValue(uint32_t c) {
    if (!IsBinaryDigit(c)) {
      return -1;
    }
    return c - '0';
  }
  static int GetDecimalValue(uint32_t c) {
    if (!IsAsciiDigit(c)) {
      return -1;
    }
    return c - '0';
  }

  static bool IsWhitespace(uint32_t c) {
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
    // '\0' does not return a defined value.
    bool carry;
    uint32_t dummy;
    const uint32_t bitmask = 0x8000'1f00;
    asm("lsrs %1, %2, %3"
        : "=@cccs"(carry), "=r"(dummy)
        : "r"(bitmask), "r"(c));
    return carry;
#else
    return c == ' ' || (9 <= c && c <= 13);
#endif
  }

  static bool IsWordCharacter(uint32_t c);
};

//---------------------------------------------------------------------------

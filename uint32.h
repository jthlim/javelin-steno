//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct Uint32 {
  Uint32() = default;

  constexpr Uint32(uint32_t value)
      : b{(uint8_t)(value), (uint8_t)(value >> 8), (uint8_t)(value >> 16),
          (uint8_t)(value >> 24)} {}

  uint32_t ToUint32() const {
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
  }

  uint8_t b[4];
};

static_assert(alignof(Uint32) == 1);
static_assert(sizeof(Uint32) == 4);
static_assert(sizeof(Uint32[2]) == 8);

//---------------------------------------------------------------------------

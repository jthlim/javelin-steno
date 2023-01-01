//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct Uint24 {
  static Uint24 Create(uint32_t v) {
    return Uint24{.b = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16)}};
  }

  uint32_t ToUint32() const { return b[0] + (b[1] << 8) + (b[2] << 16); }

  uint8_t b[3];
};

static_assert(sizeof(Uint24) == 3);
static_assert(sizeof(Uint24[2]) == 6);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct Uint16 {
  uint32_t ToUint32() const { return b[0] | (b[1] << 8); }

  uint8_t b[2];
};

static_assert(alignof(Uint16) == 1);
static_assert(sizeof(Uint16) == 2);
static_assert(sizeof(Uint16[2]) == 4);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Crc32 {
public:
  static uint32_t Hash(const void *p, size_t count);
  static consteval uint32_t EmptyHash() { return 0; }
};

//---------------------------------------------------------------------------

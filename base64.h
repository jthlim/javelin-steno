//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct Base64 {
  static bool IsValid(const char *p);
  static size_t Decode(uint8_t *destination, const uint8_t *source);
};

//---------------------------------------------------------------------------
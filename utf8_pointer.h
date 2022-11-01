//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class Utf8Pointer {
public:
  Utf8Pointer() = default;
  Utf8Pointer(const char *p) : p((uint8_t *)p) {}

  uint32_t operator*() const;
  void operator++() { p += DECODE_TABLE[*p]; }

  size_t Set(uint32_t c);
  void SetAndAdvance(uint32_t c);
  static uint32_t BytesForCharacterCode(uint32_t c);

  bool operator<(const char *other) const { return p < (const uint8_t *)other; }

private:
  uint8_t *p;

  static const uint8_t DECODE_TABLE[];
};

//---------------------------------------------------------------------------
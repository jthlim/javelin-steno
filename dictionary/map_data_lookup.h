//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class MapDataLookup {
public:
  MapDataLookup() = default;
  MapDataLookup(const uint8_t *p) : p(p) {}

  bool HasData() const { return *p != 0xff; }
  const void *GetData(const uint8_t *baseAddress) const {
    uint32_t offset = p[0] | (p[1] << 7) | (p[2] << 14) + (p[3] << 21);
    return baseAddress + offset;
  }

  void operator++() { p += 4; }

  const uint8_t *GetPointer() const { return p; }

  static const uint8_t *FindNextWordStart(const uint8_t *p) {
    while (*p != 0xff) {
      p += 4;
    }
    return p + 1;
  }

private:
  const uint8_t *p;
};

//---------------------------------------------------------------------------

struct StenoTextBlock {
  static const uint8_t *FindWordStart(const uint8_t *p) {
    while (p[-1] != 0xff) {
      --p;
    }
    return p;
  }
  static const uint8_t *FindNextWordStart(const uint8_t *p) {
    while (*p++ != 0xff) {
    }
    return p;
  }
};

//---------------------------------------------------------------------------

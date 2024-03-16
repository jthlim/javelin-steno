//---------------------------------------------------------------------------

#pragma once
#include "../xip_pointer.h"
#include <stdint.h>

//---------------------------------------------------------------------------

class MapDataLookup {
public:
  MapDataLookup() = default;
  MapDataLookup(const uint8_t *p) : p(p) {}

  bool HasData() const { return *XipPointer(p) != 0xff; }
  const void *GetData(const uint8_t *baseAddress) const {
    const uint8_t *d = XipPointer(p);
    uint32_t offset = d[0] | (d[1] << 7) | (d[2] << 14) + (d[3] << 21);
    return baseAddress + offset;
  }

  void operator++() { p += 4; }

  const uint8_t *GetPointer() const { return p; }

private:
  const uint8_t *p;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  uint32_t To565() const {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
  }

  // brightness is 0-255 representing 0-100%.
  Color Scale(int brightness) const {
    // ceil((1 << 24) / 255) = 65794
    const uint32_t scale = 65794 * brightness;
    return Color{
        .r = uint8_t(r * scale >> 24),
        .g = uint8_t(g * scale >> 24),
        .b = uint8_t(b * scale >> 24),
    };
  }
};

//---------------------------------------------------------------------------

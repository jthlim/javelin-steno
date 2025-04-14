//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  static Color From565(uint32_t v) {
    return Color{
        .r = uint8_t((v >> 11) << 3),
        .g = uint8_t(((v >> 5) & 0x3f) << 2),
        .b = uint8_t((v & 0x1f) << 3),
    };
  }

  uint32_t To565() const {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
  }

  Color Blend(const Color &other, uint32_t alpha) {
    const uint32_t y = 258 * alpha;
    const uint32_t x = 65790 - y;
    return Color{
        .r = uint8_t((r * x + other.r * y) >> 16),
        .g = uint8_t((g * x + other.g * y) >> 16),
        .b = uint8_t((b * x + other.b * y) >> 16),
    };
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

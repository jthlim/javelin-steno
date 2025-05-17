//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum class FontId : uint32_t {
  DEFAULT,
  LARGE,
  DOS,
  SMALL_DIGITS,
  MEDIUM_DIGITS,
  LARGE_DIGITS,
  HUGE_DIGITS,

  COUNT,
};

struct Font {
  uint16_t startUnicode; // Inclusive
  uint16_t endUnicode;   // Exclusive
  uint8_t width;         // 0 = proportional
  uint8_t height;        // Must be non-zero
  uint8_t baseline;      // y offset is adjusted by this value.
  uint8_t spacing;

  const uint16_t *glyphOffsets;

  // Glyph data has a width per character
  const uint8_t *glyphData;

  uint32_t GetCharacterWidth(uint32_t c) const;
  const uint8_t *GetCharacterData(uint32_t c) const;
  uint32_t GetStringWidth(const char *p) const;

  static const Font *GetFont(FontId fontId) {
    return (size_t)fontId < (size_t)FontId::COUNT ? FONTS[(size_t)fontId]
                                                  : &DEFAULT;
  }

  static const Font DEFAULT;
  static const Font LARGE;
  static const Font DOS;
  static const Font SMALL_DIGITS;
  static const Font MEDIUM_DIGITS;
  static const Font LARGE_DIGITS;
  static const Font HUGE_DIGITS;

private:
  static const Font *const FONTS[];
};

//---------------------------------------------------------------------------

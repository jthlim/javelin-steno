//---------------------------------------------------------------------------

#include "font.h"
#include "../../console.h"
#include "../../str.h"
#include "../../utf8_pointer.h"

//---------------------------------------------------------------------------

const Font *const Font::FONTS[] = {
    &Font::DEFAULT,
    &Font::SMALL_DIGITS,
    &Font::MEDIUM_DIGITS,
    &Font::LARGE_DIGITS,
};

uint32_t Font::GetCharacterWidth(uint32_t c) const {
  if (width != 0) {
    return width;
  }

  const size_t dataOffset = glyphOffsets[c - startUnicode];
  if (dataOffset == 0) {
    return 0;
  }

  return glyphData[dataOffset - 1];
}

const uint8_t *Font::GetCharacterData(uint32_t c) const {
  if (c < startUnicode || c >= endUnicode) {
    return nullptr;
  }
  size_t dataOffset = glyphOffsets[c - startUnicode];
  if (dataOffset == 0) {
    return nullptr;
  }
  if (width == 0) {
    ++dataOffset;
  }
  return &glyphData[dataOffset - 1];
}

uint32_t Font::GetStringWidth(const char *p) const {
  Utf8Pointer utf8p(p);

  uint32_t width = 0;

  for (;;) {
    const uint32_t c = *utf8p++;

    if (c == 0) {
      return width;
    }

    if (width != 0) {
      // Add spacing between characters.
      width += spacing;
    }

    width += GetCharacterWidth(c);
  }
}

//---------------------------------------------------------------------------

void Font::MeasureText_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No parameters specified\n\n");
    return;
  }

  int fontId;
  p = Str::ParseInteger(&fontId, p + 1, false);
  if (!p || *p != ' ') {
    Console::Printf("ERR fontId parameter missing\n\n");
    return;
  }
  ++p;

  const Font *font = Font::GetFont(fontId);
  const uint32_t width = font->GetStringWidth(p);
  Console::Printf("Width: %u\n\n", width);
}

//---------------------------------------------------------------------------

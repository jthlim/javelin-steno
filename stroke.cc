//---------------------------------------------------------------------------

#include "stroke.h"
#include "crc32.h"
#include "str.h"
#include "utf8_pointer.h"
#include <assert.h>

//---------------------------------------------------------------------------

constexpr StrokeKey EXTENDED_ENGLISH_STROKE_FORMATTER[] = {
    {'#', StrokeKeyType::MASK, 0x00000001},
    {'^', StrokeKeyType::MASK, 0x00800000},
    {'+', StrokeKeyType::MASK, 0x01000000},
    {'S', StrokeKeyType::MASK, 0x00000002},
    {'T', StrokeKeyType::MASK, 0x00000004},
    {'K', StrokeKeyType::MASK, 0x00000008},
    {'P', StrokeKeyType::MASK, 0x00000010},
    {'W', StrokeKeyType::MASK, 0x00000020},
    {'H', StrokeKeyType::MASK, 0x00000040},
    {'R', StrokeKeyType::MASK, 0x00000080},
    {'A', StrokeKeyType::MASK, 0x00000100},
    {'O', StrokeKeyType::MASK, 0x00000200},
    {'*', StrokeKeyType::MASK, 0x00000400},
    {'-', StrokeKeyType::SEPARATOR, 0x00001f00},
    {'-', StrokeKeyType::SEPARATOR_FOLLOW_MASK, 0x007fe000},
    {'E', StrokeKeyType::MASK, 0x00000800},
    {'U', StrokeKeyType::MASK, 0x00001000},
    {'F', StrokeKeyType::MASK, 0x00002000},
    {'R', StrokeKeyType::MASK, 0x00004000},
    {'P', StrokeKeyType::MASK, 0x00008000},
    {'B', StrokeKeyType::MASK, 0x00010000},
    {'L', StrokeKeyType::MASK, 0x00020000},
    {'G', StrokeKeyType::MASK, 0x00040000},
    {'T', StrokeKeyType::MASK, 0x00080000},
    {'S', StrokeKeyType::MASK, 0x00100000},
    {'D', StrokeKeyType::MASK, 0x00200000},
    {'Z', StrokeKeyType::MASK, 0x00400000},
};

constexpr StrokeKey EXTENDED_ENGLISH_STROKE_PARSER[] = {
    {'#', StrokeKeyType::MASK, 0x00000001},
    {'^', StrokeKeyType::MASK, 0x00800000},
    {'+', StrokeKeyType::MASK, 0x01000000},
    {'1', StrokeKeyType::MASK, 0x00000003},
    {'S', StrokeKeyType::MASK, 0x00000002},
    {'2', StrokeKeyType::MASK, 0x00000005},
    {'T', StrokeKeyType::MASK, 0x00000004},
    {'K', StrokeKeyType::MASK, 0x00000008},
    {'3', StrokeKeyType::MASK, 0x00000011},
    {'P', StrokeKeyType::MASK, 0x00000010},
    {'W', StrokeKeyType::MASK, 0x00000020},
    {'4', StrokeKeyType::MASK, 0x00000041},
    {'H', StrokeKeyType::MASK, 0x00000040},
    {'R', StrokeKeyType::MASK, 0x00000080},
    {'5', StrokeKeyType::MASK, 0x00000101},
    {'A', StrokeKeyType::MASK, 0x00000100},
    {'0', StrokeKeyType::MASK, 0x00000201},
    {'O', StrokeKeyType::MASK, 0x00000200},
    {'*', StrokeKeyType::MASK, 0x00000400},
    {'-', StrokeKeyType::SEPARATOR, 0x00000000},
    {'E', StrokeKeyType::MASK, 0x00000800},
    {'U', StrokeKeyType::MASK, 0x00001000},
    {'6', StrokeKeyType::MASK, 0x00002001},
    {'F', StrokeKeyType::MASK, 0x00002000},
    {'R', StrokeKeyType::MASK, 0x00004000},
    {'7', StrokeKeyType::MASK, 0x00008001},
    {'P', StrokeKeyType::MASK, 0x00008000},
    {'B', StrokeKeyType::MASK, 0x00010000},
    {'8', StrokeKeyType::MASK, 0x00020001},
    {'L', StrokeKeyType::MASK, 0x00020000},
    {'G', StrokeKeyType::MASK, 0x00040000},
    {'9', StrokeKeyType::MASK, 0x00080001},
    {'T', StrokeKeyType::MASK, 0x00080000},
    {'S', StrokeKeyType::MASK, 0x00100000},
    {'D', StrokeKeyType::MASK, 0x00200000},
    {'Z', StrokeKeyType::MASK, 0x00400000},
};

//---------------------------------------------------------------------------

FastIterable<const StrokeKey> StenoStroke::formatter =
    FastIterable<const StrokeKey>(EXTENDED_ENGLISH_STROKE_FORMATTER);
FastIterable<const StrokeKey> StenoStroke::parser =
    FastIterable<const StrokeKey>(EXTENDED_ENGLISH_STROKE_PARSER);

// This is placed in memory to avoid needing QSPI active to handle strokes.
List<StrokeKey> StenoStroke::formatters;
List<StrokeKey> StenoStroke::parsers;

//---------------------------------------------------------------------------

void StenoStroke::SetLanguage(const SizedList<StrokeKey> &keys) {
  parsers.Reset();
  for (const StrokeKey &key : keys) {
    switch (key.type) {
    case StrokeKeyType::MASK:
      parsers.Add(key);
      break;
    case StrokeKeyType::SEPARATOR:
      parsers.Add(StrokeKey{key.c, StrokeKeyType::SEPARATOR, 0});
      break;
    case StrokeKeyType::SEPARATOR_FOLLOW_MASK:
      break;
    }
  }
  parser = parsers;

  formatters.Reset();
  for (const StrokeKey &key : keys) {
    if (key.type != StrokeKeyType::MASK || key.IsSingleBit()) {
      formatters.Add(key);
    }
  }
  formatter = formatters;
}

//---------------------------------------------------------------------------

void StenoStroke::Set(const char *string) {
  uint32_t value = 0;
  for (const StrokeKey &display : parser) {
    if (display.c == *string) {
      value |= display.mask;
      ++string;
    }
  }

  keyState = value;
}

char *StenoStroke::ToString(char *buffer) const {
  Utf8Pointer utf8(buffer);
  const StrokeKey *pEnd = end(formatter);
  for (const StrokeKey *p = begin(formatter); p != pEnd;) {
    switch (p->type) {
    [[unlikely]] case StrokeKeyType::SEPARATOR:
      // Only draw the separator if the mask is zero, and there's some
      // characters after.
      //
      // Rather than adding 32 bits to every instance of the structure,
      // use two consecutive entries for separators.
      assert(p[1].type == StrokeKeyType::SEPARATOR_FOLLOW_MASK);
      if ((keyState & p[0].mask) == 0 && (keyState & p[1].mask) != 0) {
        utf8.SetAndAdvance(p->c);
      }
      p += 2;
      break;
    [[likely]] case StrokeKeyType::MASK:
      if ((keyState & p->mask) == p->mask) {
        utf8.SetAndAdvance(p->c);
      }
      ++p;
      break;
    default:
      __builtin_unreachable();
      break;
    }
  }

  char *result = utf8.GetRawPointer();
  *result = '\0';
  return result;
}

uint32_t StenoStroke::PopCount(const StenoStroke *strokes, size_t length) {
  uint32_t result = 0;
  for (size_t i = 0; i < length; ++i) {
    result += strokes[i].PopCount();
  }
  return result;
}

uint32_t StenoStroke::Hash(const StenoStroke *strokes, size_t length) {
  return Crc32::Hash(strokes, sizeof(StenoStroke) * length);
}

//---------------------------------------------------------------------------

#include "unit_test.h"

bool VerifyParse(const char *text, const char *expected) {
  StenoStroke stroke;
  stroke.Set(text);
  char buffer[StenoStroke::MAX_STRING_LENGTH];
  stroke.ToString(buffer);
  return Str::Eq(buffer, expected);
}

TEST_BEGIN("Stroke tests") {
  char buffer[StenoStroke::MAX_STRING_LENGTH];

  const StenoStroke hello(StrokeMask::HL | StrokeMask::LR);
  hello.ToString(buffer);
  assert(Str::Eq(buffer, "H-L"));

  VerifyParse("S", "S");
  VerifyParse("KAT", "KAT");
  VerifyParse("STA*R", "STA*R");
  VerifyParse("19", "#S-T");
  VerifyParse("^KAT", "^KAT");
}
TEST_END

//---------------------------------------------------------------------------

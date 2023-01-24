//---------------------------------------------------------------------------

#include "macos_us_unicode_data.h"
#include "key_code.h"
#include "steno_key_code.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

#define _A MODIFIER_L_ALT_FLAG
#define _S MODIFIER_L_SHIFT_FLAG

// clang-format off
// Data for typing unicode characters using "U.S." keyboard layout on macOS.
static constexpr uint16_t DATA[] = {
    0,                                                          // Marker
    0x0060, _A | _S | KeyCode::GRAVE,      0,                   // `
    0x00a1, _A | KeyCode::_0,              0,                   // ¡
    0x00a2, _A | KeyCode::_3,              0,                   // ¢
    0x00a3, _A | KeyCode::_2,              0,                   // £
    0x00a5, _A | KeyCode::Y,               0,                   // ¥
    0x00a7, _A | KeyCode::_5,              0,                   // §
    0x00a8, _A | _S | KeyCode::U,          0,                   // ¨
    0x00a9, _A | KeyCode::G,               0,                   // ©
    0x00aa, _A | KeyCode::_8,              0,                   // ª
    0x00ab, _A | KeyCode::BACKSLASH,       0,                   // «
    0x00ac, _A | KeyCode::L,               0,                   // ¬
    0x00ae, _A | KeyCode::R,               0,                   // ®
    0x00af, _A | _S | KeyCode::COMMA,      0,                   // ¯
    0x00b0, _A | _S | KeyCode::_7,         0,                   // °
    0x00b1, _A | _S | KeyCode::EQUAL,      0,                   // ±
    0x00b4, _A | _S | KeyCode::E,          0,                   // ´
    0x00b5, _A | KeyCode::M,               0,                   // µ
    0x00b6, _A | KeyCode::_6,              0,                   // ¶
    0x00b7, _A | _S | KeyCode::_8,         0,                   // ·
    0x00b8, _A | _S | KeyCode::Z,          0,                   // ¸
    0x00ba, _A | KeyCode::_9,              0,                   // º
    0x00bb, _A | _S | KeyCode::BACKSLASH,  0,                   // »
    0x00bf, _A | _S | KeyCode::SLASH,      0,                   // ¿
    0x00c0, _A | KeyCode::GRAVE,           _S | KeyCode::A, 0,  // À
    0x00c1, _A | _S | KeyCode::Y,          0,                   // Á
    0x00c1, _A | KeyCode::E,               _S | KeyCode::A, 0,  // Á
    0x00c2, _A | _S | KeyCode::M,          0,                   // Â
    0x00c2, _A | KeyCode::I,               _S | KeyCode::A, 0,  // Â
    0x00c3, _A | KeyCode::N,               _S | KeyCode::A, 0,  // Ã
    0x00c4, _A | KeyCode::U,               _S | KeyCode::A, 0,  // Ä
    0x00c5, _A | _S | KeyCode::A,          0,                   // Å
    0x00c6, _A | _S | KeyCode::APOSTROPHE, 0,                   // Æ
    0x00c7, _A | _S | KeyCode::C,          0,                   // Ç
    0x00c8, _A | KeyCode::GRAVE,           _S | KeyCode::E, 0,  // È
    0x00c9, _A | KeyCode::E,               _S | KeyCode::E, 0,  // É
    0x00ca, _A | KeyCode::I,               _S | KeyCode::E, 0,  // Ê
    0x00cb, _A | KeyCode::U,               _S | KeyCode::E, 0,  // Ë
    0x00cc, _A | KeyCode::GRAVE,           _S | KeyCode::I, 0,  // Ì
    0x00cd, _A | _S | KeyCode::S,          0,                   // Í
    0x00cd, _A | KeyCode::E,               _S | KeyCode::I, 0,  // Í
    0x00ce, _A | _S | KeyCode::D,          0,                   // Î
    0x00ce, _A | KeyCode::I,               _S | KeyCode::I, 0,  // Î
    0x00cf, _A | _S | KeyCode::F,          0,                   // Ï
    0x00cf, _A | KeyCode::U,               _S | KeyCode::I, 0,  // Ï
    0x00d1, _A | KeyCode::N,               _S | KeyCode::N, 0,  // Ñ
    0x00d2, _A | _S | KeyCode::L,          0,                   // Ò
    0x00d2, _A | KeyCode::GRAVE,           _S | KeyCode::O, 0,  // Ò
    0x00d3, _A | _S | KeyCode::H,          0,                   // Ó
    0x00d3, _A | KeyCode::E,               _S | KeyCode::O, 0,  // Ó
    0x00d4, _A | _S | KeyCode::J,          0,                   // Ô
    0x00d4, _A | KeyCode::I,               _S | KeyCode::O, 0,  // Ô
    0x00d5, _A | KeyCode::N,               _S | KeyCode::O, 0,  // Õ
    0x00d6, _A | KeyCode::U,               _S | KeyCode::O, 0,  // Ö
    0x00d8, _A | _S | KeyCode::O,          0,                   // Ø
    0x00d9, _A | KeyCode::GRAVE,           _S | KeyCode::U, 0,  // Ù
    0x00da, _A | _S | KeyCode::SEMICOLON,  0,                   // Ú
    0x00da, _A | KeyCode::E,               _S | KeyCode::U, 0,  // Ú
    0x00db, _A | KeyCode::I,               _S | KeyCode::U, 0,  // Û
    0x00dc, _A | KeyCode::U,               _S | KeyCode::U, 0,  // Ü
    0x00df, _A | KeyCode::S,               0,                   // ß
    0x00e0, _A | KeyCode::GRAVE,           KeyCode::A, 0,       // à
    0x00e1, _A | KeyCode::E,               KeyCode::A, 0,       // á
    0x00e2, _A | KeyCode::I,               KeyCode::A, 0,       // â
    0x00e3, _A | KeyCode::N,               KeyCode::A, 0,       // ã
    0x00e4, _A | KeyCode::U,               KeyCode::A, 0,       // ä
    0x00e5, _A | KeyCode::A,               0,                   // å
    0x00e6, _A | KeyCode::APOSTROPHE,      0,                   // æ
    0x00e7, _A | KeyCode::C,               0,                   // ç
    0x00e8, _A | KeyCode::GRAVE,           KeyCode::E, 0,       // è
    0x00e9, _A | KeyCode::E,               KeyCode::E, 0,       // é
    0x00ea, _A | KeyCode::I,               KeyCode::E, 0,       // ê
    0x00eb, _A | KeyCode::U,               KeyCode::E, 0,       // ë
    0x00ec, _A | KeyCode::GRAVE,           KeyCode::I, 0,       // ì
    0x00ed, _A | KeyCode::E,               KeyCode::I, 0,       // í
    0x00ee, _A | KeyCode::I,               KeyCode::I, 0,       // î
    0x00ef, _A | KeyCode::U,               KeyCode::I, 0,       // ï
    0x00f1, _A | KeyCode::N,               KeyCode::N, 0,       // ñ
    0x00f2, _A | KeyCode::GRAVE,           KeyCode::O, 0,       // ò
    0x00f3, _A | KeyCode::E,               KeyCode::O, 0,       // ó
    0x00f4, _A | KeyCode::I,               KeyCode::O, 0,       // ô
    0x00f5, _A | KeyCode::N,               KeyCode::O, 0,       // õ
    0x00f6, _A | KeyCode::U,               KeyCode::O, 0,       // ö
    0x00f7, _A | KeyCode::SLASH,           0,                   // ÷
    0x00f8, _A | KeyCode::O,               0,                   // ø
    0x00f9, _A | KeyCode::GRAVE,           KeyCode::U, 0,       // ù
    0x00fa, _A | KeyCode::E,               KeyCode::U, 0,       // ú
    0x00fb, _A | KeyCode::I,               KeyCode::U, 0,       // û
    0x00fc, _A | KeyCode::U,               KeyCode::U, 0,       // ü
    0x00ff, _A | KeyCode::U,               KeyCode::Y, 0,       // ÿ
    0x0131, _A | _S | KeyCode::B,          0,                   // ı
    0x0152, _A | _S | KeyCode::Q,          0,                   // Œ
    0x0153, _A | KeyCode::Q,               0,                   // œ
    0x0178, _A | KeyCode::U,               _S | KeyCode::Y, 0,  // Ÿ
    0x0192, _A | KeyCode::F,               0,                   // ƒ
    0x02c6, _A | _S | KeyCode::I,          0,                   // ˆ
    0x02c7, _A | _S | KeyCode::T,          0,                   // ˇ
    0x02d8, _A | _S | KeyCode::DOT,        0,                   // ˘
    0x02d9, _A | KeyCode::H,               0,                   // ˙
    0x02da, _A | KeyCode::K,               0,                   // ˚
    0x02db, _A | _S | KeyCode::X,          0,                   // ˛
    0x02dc, _A | _S | KeyCode::N,          0,                   // ˜
    0x02dd, _A | _S | KeyCode::G,          0,                   // ˝
    0x03a9, _A | KeyCode::Z,               0,                   // Ω
    0x03c0, _A | KeyCode::P,               0,                   // π
    0x2013, _A | KeyCode::MINUS,           0,                   // –
    0x2014, _A | _S | KeyCode::MINUS,      0,                   // —
    0x2018, _A | KeyCode::R_BRACKET,       0,                   // ‘
    0x2019, _A | _S | KeyCode::R_BRACKET,  0,                   // ’
    0x201a, _A | _S | KeyCode::_9,         0,                   // ‚
    0x201c, _A | KeyCode::L_BRACKET,       0,                   // “
    0x201d, _A | _S | KeyCode::L_BRACKET,  0,                   // ”
    0x201e, _A | _S | KeyCode::W,          0,                   // „
    0x2020, _A | KeyCode::T,               0,                   // †
    0x2021, _A | _S | KeyCode::_6,         0,                   // ‡
    0x2022, _A | KeyCode::_7,              0,                   // •
    0x2026, _A | KeyCode::SEMICOLON,       0,                   // …
    0x2030, _A | _S | KeyCode::R,          0,                   // ‰
    0x2039, _A | _S | KeyCode::_2,         0,                   // ‹
    0x203a, _A | _S | KeyCode::_3,         0,                   // ›
    0x2044, _A | _S | KeyCode::_0,         0,                   // ⁄
    0x20ac, _A | _S | KeyCode::_1,         0,                   // €
    0x2122, _A | KeyCode::_1,              0,                   // ™
    0x2202, _A | KeyCode::D,               0,                   // ∂
    0x2206, _A | KeyCode::J,               0,                   // ∆
    0x220f, _A | _S | KeyCode::P,          0,                   // ∏
    0x2211, _A | KeyCode::W,               0,                   // ∑
    0x221a, _A | KeyCode::V,               0,                   // √
    0x221e, _A | KeyCode::_4,              0,                   // ∞
    0x222b, _A | KeyCode::B,               0,                   // ∫
    0x2248, _A | KeyCode::X,               0,                   // ≈
    0x2260, _A | KeyCode::EQUAL,           0,                   // ≠
    0x2264, _A | KeyCode::COMMA,           0,                   // ≤
    0x2265, _A | KeyCode::DOT,             0,                   // ≥
    0x25ca, _A | _S | KeyCode::V,          0,                   // ◊
    0xf8ff, _A | _S | KeyCode::K,          0,                   // 
    0xfb01, _A | _S | KeyCode::_4,         0,                   // ﬁ
    0xfb02, _A | _S | KeyCode::_5,         0,                   // ﬂ
    0,
};
// clang-format on

//---------------------------------------------------------------------------

const uint16_t *MacOsUsUnicodeData::GetSequenceForUnicode(uint32_t unicode) {
  const uint16_t *left = DATA + 1;
  const uint16_t *right = DATA + sizeof(DATA) / sizeof(*DATA) - 1;

  while (left < right) {
#if JAVELIN_PLATFORM_PICO_SDK
    // Optimization when top bit of pointer cannot be set.
    const uint16_t *mid =
        (const uint16_t *)((size_t(left) + size_t(right)) / 2);
#else
    const uint16_t *mid = left + size_t(right - left) / 2;
#endif

    const uint16_t *entryStart = mid;
    while (entryStart[-1] != 0) {
      --entryStart;
    }

    int compare = (int)unicode - (int)*entryStart;
    if (compare < 0) {
      right = entryStart;
    } else if (compare == 0) {
      return entryStart + 1;
    } else {
      const uint16_t *entryEnd = mid;
      while (*entryEnd) {
        ++entryEnd;
      }

      left = entryEnd + 1;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"
#include <assert.h>

TEST_BEGIN("MacOsUsUnicodeData tests") {
  assert(MacOsUsUnicodeData::GetSequenceForUnicode(0x59) == nullptr);
  assert(MacOsUsUnicodeData::GetSequenceForUnicode(0x60)[-1] == 0x60);
  assert(MacOsUsUnicodeData::GetSequenceForUnicode(0x61) == nullptr);
  assert(MacOsUsUnicodeData::GetSequenceForUnicode(0xfb02)[-1] == 0xfb02);
  assert(MacOsUsUnicodeData::GetSequenceForUnicode(0xfb03) == nullptr);
}
TEST_END

//---------------------------------------------------------------------------

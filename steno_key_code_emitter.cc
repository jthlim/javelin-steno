//---------------------------------------------------------------------------

#include "steno_key_code_emitter.h"
#include "key_code.h"
#include "macos_us_unicode_data.h"
#include "steno_key_code.h"
#include "str.h"
#include "windows_alt_unicode_data.h"

//---------------------------------------------------------------------------

UnicodeMode StenoKeyCodeEmitter::emitterMode = UnicodeMode::MACOS_US;

//---------------------------------------------------------------------------

struct StenoKeyCodeEmitter::EmitterContext {
  EmitterContext(bool shouldStoreStroke)
      : shouldStoreStroke(shouldStoreStroke) {}
  uint32_t modifiers = 0;
  bool shouldStoreStroke;
  bool hasDeterminedNumLockState = false;
  bool isNumLockOn;

  static const uint8_t MASK_KEY_CODES[];
  static const uint16_t ALT_HEX_KEY_CODES[];
  static const uint16_t KP_ALT_HEX_KEY_CODES[];
  static const uint16_t ASCII_KEY_CODES[];

  bool GetIsNumLockOn() {
    if (!hasDeterminedNumLockState) {
      hasDeterminedNumLockState = true;
      isNumLockOn = Key::IsNumLockOn();
    }
    return isNumLockOn;
  }

  void ProcessStenoKeyCode(StenoKeyCode stenoKeyCode);

  static void PressKey(int keyCode) { Key::Press(keyCode); }
  static void ReleaseKey(int keyCode) { Key::Release(keyCode); };

  static void TapKey(int keyCode) {
    PressKey(keyCode);
    ReleaseKey(keyCode);
  }

  void EmitKeyCode(uint32_t keyCode);

  static void PressModifiers(uint32_t modifiers);
  static void ReleaseModifiers(uint32_t modifiers);

  void EmitNonAscii(uint32_t unicode);
  void EmitMacOsUs(uint32_t unicode);
  void EmitMacOsUnicodeHex(uint32_t unicode);
  void EmitWindowsAlt(uint32_t unicode);
  void RecurseEmitWindowsAlt(int alt);
  void EmitWindowsHex(uint32_t unicode);
  void EmitUCS2AltHex(uint32_t unicode);
};

const char *const StenoKeyCodeEmitter::UNICODE_EMITTER_NAMES[] = {
    "none", "macos_us", "macos_hex", "windows_alt", "windows_hex",
};

const char *UnicodeModeName(UnicodeMode mode) {
  uint8_t index = (uint8_t)mode;
  if (index > (uint8_t)UnicodeMode::COUNT) {
    index = 0;
  }
  return StenoKeyCodeEmitter::UNICODE_EMITTER_NAMES[index];
}

bool StenoKeyCodeEmitter::SetUnicodeMode(const char *name) {
  for (size_t i = 0; i < (size_t)UnicodeMode::COUNT; ++i) {
    if (Str::Eq(name, StenoKeyCodeEmitter::UNICODE_EMITTER_NAMES[i])) {
      emitterMode = (UnicodeMode)i;
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------

const uint8_t StenoKeyCodeEmitter::EmitterContext::MASK_KEY_CODES[] = {
    KeyCode::L_CTRL, KeyCode::L_SHIFT, KeyCode::L_ALT, KeyCode::L_META,
    KeyCode::R_CTRL, KeyCode::R_SHIFT, KeyCode::R_ALT, KeyCode::R_META,
};

#define M MODIFIER_L_ALT_FLAG
const uint16_t StenoKeyCodeEmitter::EmitterContext::ALT_HEX_KEY_CODES[] = {
    M | KeyCode::_0, M | KeyCode::_1, M | KeyCode::_2, M | KeyCode::_3,
    M | KeyCode::_4, M | KeyCode::_5, M | KeyCode::_6, M | KeyCode::_7,
    M | KeyCode::_8, M | KeyCode::_9, M | KeyCode::A,  M | KeyCode::B,
    M | KeyCode::C,  M | KeyCode::D,  M | KeyCode::E,  M | KeyCode::F,
};

const uint16_t StenoKeyCodeEmitter::EmitterContext::KP_ALT_HEX_KEY_CODES[] = {
    M | KeyCode::KP_0, M | KeyCode::KP_1, M | KeyCode::KP_2, M | KeyCode::KP_3,
    M | KeyCode::KP_4, M | KeyCode::KP_5, M | KeyCode::KP_6, M | KeyCode::KP_7,
    M | KeyCode::KP_8, M | KeyCode::KP_9, M | KeyCode::A,    M | KeyCode::B,
    M | KeyCode::C,    M | KeyCode::D,    M | KeyCode::E,    M | KeyCode::F,
};

#undef M

// clang-format off
// spellchecker: disable
#define LS MODIFIER_L_SHIFT_FLAG
#define BS BACKSPACE
#define APOS APOSTROPHE
#define SCN SEMICOLON
const uint16_t StenoKeyCodeEmitter::EmitterContext::ASCII_KEY_CODES[] = {
  // NUL       SOH           STX             ETX           EOT    ENQ    ACK    BEL
  0,           0,            0,              0,            0,     0,     0,     0,
  // BS        TAB           LF              VT            FF     CR     SO     SI
  KeyCode::BS, KeyCode::TAB, KeyCode::ENTER, 0,            0,     0,     0,     0,
  // DLE       DC1           DC2             DC3           DC4    NAK    SYN    ETB
  0,           0,            0,              0,            0,     0,     0,     0,
  // CAN       EM            SUB             ESC           FS     GS     RS     US
  0,           0,            0,              KeyCode::ESC, 0,     0,     0,     0,

  //              !               "               #               $               %               &               '
  KeyCode::SPACE, LS|KeyCode::_1, LS|KeyCode::APOS,LS|KeyCode::_3,   LS|KeyCode::_4,   LS|KeyCode::_5,   LS|KeyCode::_7,   KeyCode::APOSTROPHE,
  // (            )               *               +               ,               -               .               /
  LS|KeyCode::_9, LS|KeyCode::_0, LS|KeyCode::_8, LS|KeyCode::EQUAL, KeyCode::COMMA,  KeyCode::MINUS,  KeyCode::DOT,   KeyCode::SLASH,
  // 0            1               2               3               4               5               6               7
  KeyCode::_0,    KeyCode::_1,    KeyCode::_2,    KeyCode::_3,     KeyCode::_4,     KeyCode::_5,     KeyCode::_6,     KeyCode::_7,
  // 8            9               :               ;               <               =               >               ?
  KeyCode::_8,    KeyCode::_9,    LS|KeyCode::SCN,KeyCode::SCN,   LS|KeyCode::COMMA,KeyCode::EQUAL,   LS|KeyCode::DOT, LS|KeyCode::SLASH,
  // @            A               B               C               D               E               F               G
  LS|KeyCode::_2, LS|KeyCode::A,  LS|KeyCode::B,  LS|KeyCode::C,  LS|KeyCode::D,   LS|KeyCode::E,   LS|KeyCode::F,   LS|KeyCode::G,
  // H            I               J               K               L               M               N               O
  LS|KeyCode::H,  LS|KeyCode::I,  LS|KeyCode::J,  LS|KeyCode::K,  LS|KeyCode::L,   LS|KeyCode::M,   LS|KeyCode::N,   LS|KeyCode::O,
  // P            Q               R               S               T               U               V               W
  LS|KeyCode::P,  LS|KeyCode::Q,  LS|KeyCode::R,  LS|KeyCode::S,  LS|KeyCode::T,   LS|KeyCode::U,   LS|KeyCode::V,   LS|KeyCode::W,
  // X            Y               Z               [               \               ]               ^               _
  LS|KeyCode::X,  LS|KeyCode::Y,  LS|KeyCode::Z,  KeyCode::L_BRACKET,  KeyCode::BACKSLASH,  KeyCode::R_BRACKET,  LS|KeyCode::_6,   LS|KeyCode::MINUS,
  // `            a               b               c               d               e               f               g
  KeyCode::GRAVE, KeyCode::A,     KeyCode::B,     KeyCode::C,     KeyCode::D,     KeyCode::E,     KeyCode::F,     KeyCode::G,
  // h            i               j               k               l               m               n               o
  KeyCode::H,     KeyCode::I,     KeyCode::J,     KeyCode::K,     KeyCode::L,     KeyCode::M,     KeyCode::N,     KeyCode::O,
  // p            q               r               s               t               u               v               w
  KeyCode::P,     KeyCode::Q,     KeyCode::R,     KeyCode::S,     KeyCode::T,     KeyCode::U,     KeyCode::V,     KeyCode::W,
  // x            y               z               {               |               }               ~               DEL
  KeyCode::X,     KeyCode::Y,     KeyCode::Z,     LS|KeyCode::L_BRACKET,LS|KeyCode::BACKSLASH,LS|KeyCode::R_BRACKET,LS|KeyCode::GRAVE, 0,
};
#undef S
// spellchecker: enable
// clang-format on

//---------------------------------------------------------------------------

bool StenoKeyCodeEmitter::Process(const StenoKeyCode *previous,
                                  size_t previousLength,
                                  const StenoKeyCode *value,
                                  size_t valueLength) const {
  // Skip common prefixes.
  while (previousLength > 0 && valueLength > 0 && *previous == *value) {
    --previousLength;
    --valueLength;
    ++previous;
    ++value;
  }

  if (previousLength == 0 && valueLength == 0)
    return false;

  EmitterContext context(previousLength != 0);

  // Now the length of previous represents how much needs to be backspaced.
  for (size_t i = 0; i < previousLength; ++i) {
    context.TapKey(KeyCode::BACKSPACE);
  }

  for (size_t i = 0; i < valueLength; ++i) {
    context.ProcessStenoKeyCode(value[i]);
  }

  context.ReleaseModifiers(context.modifiers);

  return context.shouldStoreStroke;
}

//---------------------------------------------------------------------------

void StenoKeyCodeEmitter::EmitterContext::ProcessStenoKeyCode(
    StenoKeyCode stenoKeyCode) {
  // Convert the incoming keyCode to a raw character code.
  if (stenoKeyCode.IsRawKeyCode()) {
    ReleaseModifiers(modifiers);
    modifiers = 0;

    if (stenoKeyCode.IsPress()) {
      PressKey(stenoKeyCode.GetRawKeyCode());
    } else {
      ReleaseKey(stenoKeyCode.GetRawKeyCode());
    }
  } else {
    shouldStoreStroke = true;
    uint32_t unicode = stenoKeyCode.ResolveUnicode();

    if (unicode < 128) {
      uint32_t keyCode = ASCII_KEY_CODES[unicode];
      if (keyCode != 0) {
        EmitKeyCode(keyCode);
      }
      return;
    }

    EmitNonAscii(unicode);
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitNonAscii(uint32_t unicode) {
  if (0xd800 <= unicode && unicode <= 0xdfff) {
    // Emit question mark. Can't display surrogate pairs.
    return EmitKeyCode(MODIFIER_L_SHIFT_FLAG | KeyCode::SLASH);
  }

  // Unicode point.
  switch (emitterMode) {
  case UnicodeMode::MACOS_US:
    EmitMacOsUs(unicode);
    break;

  case UnicodeMode::MACOS_UNICODE_HEX:
    EmitMacOsUnicodeHex(unicode);
    break;

  case UnicodeMode::WINDOWS_ALT:
    EmitWindowsAlt(unicode);
    break;

  case UnicodeMode::WINDOWS_HEX:
    EmitWindowsHex(unicode);
    break;

  case UnicodeMode::NONE:
  default:
    // Can't emit this keycode..
    EmitKeyCode(ASCII_KEY_CODES['?']);
    break;
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitMacOsUs(uint32_t unicode) {
  const uint16_t *sequence = MacOsUsUnicodeData::GetSequenceForUnicode(unicode);
  if (sequence == nullptr) {
    // Can't emit this keycode..
    EmitKeyCode(ASCII_KEY_CODES['?']);
    return;
  }

  while (*sequence) {
    EmitKeyCode(*sequence++);
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitMacOsUnicodeHex(
    uint32_t unicode) {
  if (unicode < 0x10000) {
    EmitUCS2AltHex(unicode);
  } else {
    unicode -= 0x10000;
    EmitUCS2AltHex(0xd800 | ((unicode >> 10) & 0x3ff));
    EmitUCS2AltHex(0xdc00 | (unicode & 0x3ff));
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitWindowsAlt(uint32_t unicode) {
  int alt = WindowsAltUnicodeData::GetAltCodeForUnicode(unicode);
  if (alt == 0) {
    EmitKeyCode(ASCII_KEY_CODES['?']);
    return;
  }

  bool isNumLockOn = GetIsNumLockOn();
  if (!isNumLockOn) {
    TapKey(KeyCode::NUM_LOCK);
  }
  RecurseEmitWindowsAlt(alt);

  ReleaseModifiers(modifiers);
  modifiers = 0;
  if (!isNumLockOn) {
    TapKey(KeyCode::NUM_LOCK);
  }
}

void StenoKeyCodeEmitter::EmitterContext::RecurseEmitWindowsAlt(int alt) {
  int quotient = alt / 10;
  int remainder = alt % 10;
  if (quotient != 0) {
    RecurseEmitWindowsAlt(quotient);
  }
  EmitKeyCode(KP_ALT_HEX_KEY_CODES[remainder]);
}

void StenoKeyCodeEmitter::EmitterContext::EmitWindowsHex(uint32_t unicode) {
  if (unicode >= 0x10000) {
    // Can't emit this keycode..
    EmitKeyCode(ASCII_KEY_CODES['?']);
    return;
  }

  bool isNumLockOn = GetIsNumLockOn();
  if (!isNumLockOn) {
    TapKey(KeyCode::NUM_LOCK);
  }
  EmitKeyCode(MODIFIER_L_ALT_FLAG | KeyCode::KP_PLUS);
  for (int i = 12; i >= 0; i -= 4) {
    EmitKeyCode(KP_ALT_HEX_KEY_CODES[(unicode >> i) & 0xf]);
  }
  ReleaseModifiers(modifiers);
  modifiers = 0;
  if (!isNumLockOn) {
    TapKey(KeyCode::NUM_LOCK);
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitUCS2AltHex(uint32_t unicode) {
  for (int i = 0; i < 4; ++i) {
    EmitKeyCode(ALT_HEX_KEY_CODES[(unicode >> (12 - 4 * i)) & 0xf]);
  }
}

__attribute__((weak)) void
StenoKeyCodeEmitter::EmitterContext::EmitKeyCode(uint32_t keyCode) {
  ReleaseModifiers(modifiers & ~keyCode);
  PressModifiers((keyCode & ~modifiers) & MODIFIER_MASK);
  modifiers = keyCode & MODIFIER_MASK;
  TapKey(keyCode & 0xff);
}

void StenoKeyCodeEmitter::EmitterContext::PressModifiers(uint32_t modifiers) {
  while (modifiers != 0) {
    PressKey(MASK_KEY_CODES[__builtin_ctzl(modifiers) - MODIFIER_BIT_SHIFT]);

    // Zero the lowest bit.
    modifiers &= modifiers - 1;
  }
}

void StenoKeyCodeEmitter::EmitterContext::ReleaseModifiers(uint32_t modifiers) {
  while (modifiers != 0) {
    ReleaseKey(MASK_KEY_CODES[__builtin_ctzl(modifiers) - MODIFIER_BIT_SHIFT]);

    // Zero the lowest bit.
    modifiers &= modifiers - 1;
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifdef RUN_TESTS

#include "unit_test.h"
#include <stdio.h>

#define assert_begin() size_t index = 0

#define assert_end()                                                           \
  assert(Key::history.size() == index);                                        \
  Key::history.clear()

#define assert_press(x)                                                        \
  assert(Key::history[index].isPress);                                         \
  assert(Key::history[index].code == x);                                       \
  index++

#define assert_release(x)                                                      \
  assert(!Key::history[index].isPress);                                        \
  assert(Key::history[index].code == x);                                       \
  index++

#define assert_tap(x)                                                          \
  assert_press(x);                                                             \
  assert_release(x)

TEST_BEGIN("StenoKeyCodeEmitter: Basic tests") {
  StenoKeyCodeEmitter emitter;

  StenoKeyCode codes[] = {
      StenoKeyCode('c', StenoCaseMode::TITLE),
      StenoKeyCode('a', StenoCaseMode::NORMAL),
      StenoKeyCode('t', StenoCaseMode::NORMAL),
  };

  emitter.Process(nullptr, 0, codes, 3);

  assert_begin();
  assert_press(KeyCode::L_SHIFT);
  assert_tap(KeyCode::C);
  assert_release(KeyCode::L_SHIFT);
  assert_tap(KeyCode::A);
  assert_tap(KeyCode::T);
  assert_end();
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: Backspace tests") {
  StenoKeyCodeEmitter emitter;

  StenoKeyCode previous[] = {
      StenoKeyCode('c', StenoCaseMode::TITLE),
      StenoKeyCode('o', StenoCaseMode::NORMAL),
      StenoKeyCode('g', StenoCaseMode::NORMAL),
  };
  StenoKeyCode codes[] = {
      StenoKeyCode('c', StenoCaseMode::TITLE),
      StenoKeyCode('a', StenoCaseMode::NORMAL),
      StenoKeyCode('t', StenoCaseMode::NORMAL),
  };

  emitter.Process(previous, 3, codes, 3);

  assert_begin();
  assert_tap(KeyCode::BACKSPACE);
  assert_tap(KeyCode::BACKSPACE);
  assert_tap(KeyCode::A);
  assert_tap(KeyCode::T);
  assert_end();
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: Shared modifier tests") {
  StenoKeyCodeEmitter emitter;

  StenoKeyCode codes[] = {
      StenoKeyCode('c', StenoCaseMode::UPPER),
      StenoKeyCode('a', StenoCaseMode::UPPER),
      StenoKeyCode('t', StenoCaseMode::UPPER),
  };

  emitter.Process(nullptr, 0, codes, 3);

  assert_begin();
  assert_press(KeyCode::L_SHIFT);
  assert_tap(KeyCode::C);
  assert_tap(KeyCode::A);
  assert_tap(KeyCode::T);
  assert_release(KeyCode::L_SHIFT);
  assert_end();
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: MacOS Unicode Hex test") {
  StenoKeyCodeEmitter emitter;
  emitter.SetUnicodeMode(UnicodeMode::MACOS_UNICODE_HEX);

  StenoKeyCode codes[] = {
      StenoKeyCode(0x4f60, StenoCaseMode::NORMAL),  // '你'
      StenoKeyCode(0x597d, StenoCaseMode::NORMAL),  // '好'
      StenoKeyCode(0x1f600, StenoCaseMode::NORMAL), // '😀'
      StenoKeyCode(0x1f60e, StenoCaseMode::NORMAL), // '😎'
  };

  emitter.Process(nullptr, 0, codes, 4);

  assert_begin();
  assert_press(KeyCode::L_ALT);

  assert_tap(KeyCode::_4);
  assert_tap(KeyCode::F);
  assert_tap(KeyCode::_6);
  assert_tap(KeyCode::_0);

  assert_tap(KeyCode::_5);
  assert_tap(KeyCode::_9);
  assert_tap(KeyCode::_7);
  assert_tap(KeyCode::D);

  assert_tap(KeyCode::D);
  assert_tap(KeyCode::_8);
  assert_tap(KeyCode::_3);
  assert_tap(KeyCode::D);
  assert_tap(KeyCode::D);
  assert_tap(KeyCode::E);
  assert_tap(KeyCode::_0);
  assert_tap(KeyCode::_0);

  assert_tap(KeyCode::D);
  assert_tap(KeyCode::_8);
  assert_tap(KeyCode::_3);
  assert_tap(KeyCode::D);
  assert_tap(KeyCode::D);
  assert_tap(KeyCode::E);
  assert_tap(KeyCode::_0);
  assert_tap(KeyCode::E);

  assert_release(KeyCode::L_ALT);
  assert_end();
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: Windows Alt test") {
  StenoKeyCodeEmitter emitter;
  emitter.SetUnicodeMode(UnicodeMode::WINDOWS_ALT);

  StenoKeyCode codes[] = {
      StenoKeyCode('a', StenoCaseMode::NORMAL),
      StenoKeyCode(u'Ä', StenoCaseMode::NORMAL), // Alt code 142
      StenoKeyCode(u'É', StenoCaseMode::NORMAL), // Alt code 144
      StenoKeyCode('e', StenoCaseMode::NORMAL),
  };

  emitter.Process(nullptr, 0, codes, 4);

  assert_begin();
  assert_tap(KeyCode::A);

  assert_tap(KeyCode::NUM_LOCK);
  assert_press(KeyCode::L_ALT);
  assert_tap(KeyCode::KP_1);
  assert_tap(KeyCode::KP_4);
  assert_tap(KeyCode::KP_2);
  assert_release(KeyCode::L_ALT);
  assert_tap(KeyCode::NUM_LOCK);

  assert_tap(KeyCode::NUM_LOCK);
  assert_press(KeyCode::L_ALT);
  assert_tap(KeyCode::KP_1);
  assert_tap(KeyCode::KP_4);
  assert_tap(KeyCode::KP_4);
  assert_release(KeyCode::L_ALT);
  assert_tap(KeyCode::NUM_LOCK);

  assert_tap(KeyCode::E);
  assert_end();
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: Windows Hex test") {
  StenoKeyCodeEmitter emitter;
  emitter.SetUnicodeMode(UnicodeMode::WINDOWS_HEX);

  StenoKeyCode codes[] = {
      StenoKeyCode('a', StenoCaseMode::NORMAL),
      StenoKeyCode(u'Ä', StenoCaseMode::NORMAL), // Alt code 142
      StenoKeyCode(u'É', StenoCaseMode::NORMAL), // Alt code 144
      StenoKeyCode('e', StenoCaseMode::NORMAL),
  };

  emitter.Process(nullptr, 0, codes, 4);

  assert_begin();
  assert_tap(KeyCode::A);

  assert_tap(KeyCode::NUM_LOCK);
  assert_press(KeyCode::L_ALT);
  assert_tap(KeyCode::KP_PLUS);
  assert_tap(KeyCode::KP_0);
  assert_tap(KeyCode::KP_0);
  assert_tap(KeyCode::C);
  assert_tap(KeyCode::KP_4);
  assert_release(KeyCode::L_ALT);
  assert_tap(KeyCode::NUM_LOCK);

  assert_tap(KeyCode::NUM_LOCK);
  assert_press(KeyCode::L_ALT);
  assert_tap(KeyCode::KP_PLUS);
  assert_tap(KeyCode::KP_0);
  assert_tap(KeyCode::KP_0);
  assert_tap(KeyCode::C);
  assert_tap(KeyCode::KP_9);
  assert_release(KeyCode::L_ALT);
  assert_tap(KeyCode::NUM_LOCK);

  assert_tap(KeyCode::E);
  assert_end();
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: MacOS US Unicode test") {
  StenoKeyCodeEmitter emitter;
  emitter.SetUnicodeMode(UnicodeMode::MACOS_US);

  StenoKeyCode codes[] = {
      StenoKeyCode(0x00c4, StenoCaseMode::NORMAL), // 'Ä'
  };

  emitter.Process(nullptr, 0, codes, 1);

  assert_begin();
  assert_press(KeyCode::L_ALT);
  assert_tap(KeyCode::U);
  assert_release(KeyCode::L_ALT);
  assert_press(KeyCode::L_SHIFT);
  assert_tap(KeyCode::A);
  assert_release(KeyCode::L_SHIFT);
  assert_end();
}
TEST_END

#endif

//---------------------------------------------------------------------------

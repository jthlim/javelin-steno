//---------------------------------------------------------------------------

#include "steno_key_code_emitter.h"
#include "hal/connection.h"
#include "host_layout.h"
#include "key.h"
#include "keyboard_led_status.h"
#include "steno_key_code.h"
#include "steno_key_code_emitter_context.h"

//---------------------------------------------------------------------------

const KeyCode::Value StenoKeyCodeEmitter::EmitterContext::MASK_KEY_CODES[] = {
    KeyCode::L_CTRL, KeyCode::L_SHIFT, KeyCode::L_ALT, KeyCode::L_META,
    KeyCode::R_CTRL, KeyCode::R_SHIFT, KeyCode::R_ALT, KeyCode::R_META,
};

const KeyCode::Value StenoKeyCodeEmitter::EmitterContext::HEX_KEY_CODES[] = {
    KeyCode::_0, KeyCode::_1, KeyCode::_2, KeyCode::_3,
    KeyCode::_4, KeyCode::_5, KeyCode::_6, KeyCode::_7,
    KeyCode::_8, KeyCode::_9, KeyCode::A,  KeyCode::B,
    KeyCode::C,  KeyCode::D,  KeyCode::E,  KeyCode::F,
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

//---------------------------------------------------------------------------

StenoKeyCodeEmitter::EmitterContext::EmitterContext()
    : hostLayout(HostLayouts::GetActiveLayout()) {}

bool StenoKeyCodeEmitter::EmitterContext::GetIsNumLockOn() {
  if (!hasDeterminedNumLockState) {
    hasDeterminedNumLockState = true;
    isNumLockOn = Connection::GetActiveKeyboardLedStatus().IsNumLockOn();
  }
  return isNumLockOn;
}

//---------------------------------------------------------------------------

bool StenoKeyCodeEmitter::Process(const StenoKeyCode *previous,
                                  size_t previousLength,
                                  const StenoKeyCode *value,
                                  size_t valueLength) const {
  // Skip common prefixes.
  while (previousLength > 0 && valueLength > 0 &&
         previous->HasSameOutput(*value)) {
    --previousLength;
    --valueLength;
    ++previous;
    ++value;
  }

  if (previousLength == 0 && valueLength == 0) {
    return true;
  }

  EmitterContext context;

  // Now the length of previous represents how much needs to be backspaced.
  for (size_t i = 0; i < previousLength; ++i) {
    if (!previous[i].IsRawKeyCode()) {
      context.shouldCombineUndo = false;
      context.TapKey(KeyCode::BACKSPACE);
    }
  }

  for (size_t i = 0; i < valueLength; ++i) {
    context.ProcessStenoKeyCode(value[i]);
  }

  context.ReleaseModifiers(context.modifiers);

  return context.shouldCombineUndo;
}

void StenoKeyCodeEmitter::Emit(const StenoKeyCode *value, size_t length) const {
  EmitterContext context;

  for (size_t i = 0; i < length; ++i) {
    context.ProcessStenoKeyCode(value[i]);
  }

  context.ReleaseModifiers(context.modifiers);
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
    shouldCombineUndo = false;
    const uint32_t unicode = stenoKeyCode.ResolveOutputUnicode();

    if (unicode < 128) {
      EmitAscii(unicode);
    } else {
      EmitNonAscii(unicode);
    }
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitAscii(uint32_t unicode) {
  assert(unicode < 128);
  const uint32_t keyCode = hostLayout.asciiKeyCodes[unicode];
  if (keyCode == 0) {
    return;
  }
  if (keyCode == 0xffff) {
    return EmitNonAscii(unicode);
  }
  EmitKeyCode(keyCode);
}

void StenoKeyCodeEmitter::EmitterContext::EmitNonAscii(uint32_t unicode) {
  if (0xd800 <= unicode && unicode <= 0xdfff) {
    // Emit question mark. Can't display surrogate pairs.
    return EmitAscii('?');
  }

  const HostLayoutEntry *sequence = hostLayout.GetSequenceForUnicode(unicode);
  if (sequence != nullptr) {
    return EmitSequence(*sequence);
  }

  // Unicode point.
  switch (hostLayout.unicodeMode) {
  case UnicodeMode::MACOS_UNICODE_HEX:
    return EmitMacOsUnicodeHex(unicode);

  case UnicodeMode::WINDOWS_HEX:
    return EmitWindowsHex(unicode);

  case UnicodeMode::LINUX_IBUS:
    return EmitIBus(unicode);

  case UnicodeMode::NONE:
  default:
    // Can't emit this keycode..
    return EmitAscii('?');
  }
}

void StenoKeyCodeEmitter::EmitterContext::EmitSequence(
    const HostLayoutEntry &sequence) {
  for (size_t i = 0; i < sequence.length; ++i) {
    EmitKeyCode(sequence.keyCodes[i]);
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

void StenoKeyCodeEmitter::EmitterContext::EmitIBus(uint32_t unicode) {
  const uint32_t uKeyCode = hostLayout.asciiKeyCodes[uint32_t('u')];

  EmitKeyCode(MODIFIER_L_CTRL_FLAG | MODIFIER_L_SHIFT_FLAG | uKeyCode);
  RecurseEmitIBus(unicode);
  EmitAscii('\n');
  EmitIBusDelay();
}

void StenoKeyCodeEmitter::EmitterContext::EmitIBusDelay() {
  Key::Flush();
  Key::Flush();
}

[[gnu::noinline]] void
StenoKeyCodeEmitter::EmitterContext::RecurseEmitIBus(uint32_t code) {
  const uint32_t quotient = code / 16;
  const uint32_t remainder = code % 16;
  if (quotient != 0) {
    RecurseEmitIBus(quotient);
  }
  EmitAscii("0123456789abcdef"[remainder]);
}

void StenoKeyCodeEmitter::EmitterContext::EmitWindowsHex(uint32_t unicode) {
  if (unicode >= 0x10000) {
    // Can't emit this keycode..
    EmitAscii('?');
    return;
  }

  const bool isNumLockOn = GetIsNumLockOn();
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

[[gnu::weak]] void
StenoKeyCodeEmitter::EmitterContext::EmitKeyCode(uint32_t keyCode) {
  ReleaseModifiers(modifiers & ~keyCode);
  PressModifiers((keyCode & ~modifiers) & MODIFIER_MASK);
  modifiers = keyCode & MODIFIER_MASK;
  TapKey(KeyCode::Value(keyCode & 0xff));
}

void StenoKeyCodeEmitter::EmitterContext::PressModifiers(uint32_t modifiers) {
  if (modifiers == 0) {
    return;
  }
  if (hostLayout.unicodeMode == UnicodeMode::LINUX_IBUS) {
    EmitIBusDelay();
  }
  while (modifiers != 0) {
    PressKey(MASK_KEY_CODES[__builtin_ctzl(modifiers) - MODIFIER_BIT_SHIFT]);

    // Zero the lowest bit.
    modifiers &= modifiers - 1;
  }
}

void StenoKeyCodeEmitter::EmitterContext::ReleaseModifiers(uint32_t modifiers) {
  if (modifiers == 0) {
    return;
  }
  if (hostLayout.unicodeMode == UnicodeMode::LINUX_IBUS) {
    EmitIBusDelay();
  }
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

  const StenoKeyCode codes[] = {
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

  const StenoKeyCode previous[] = {
      StenoKeyCode('c', StenoCaseMode::TITLE),
      StenoKeyCode('o', StenoCaseMode::NORMAL),
      StenoKeyCode('g', StenoCaseMode::NORMAL),
  };
  const StenoKeyCode codes[] = {
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

  const StenoKeyCode codes[] = {
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
  HostLayout layout = HostLayout::ansi;
  layout.unicodeMode = UnicodeMode::MACOS_UNICODE_HEX;
  HostLayouts::SetActiveLayout(layout);
  StenoKeyCodeEmitter emitter;

  const StenoKeyCode codes[] = {
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

  HostLayouts::SetActiveLayout(HostLayout::ansi);
}
TEST_END

TEST_BEGIN("StenoKeyCodeEmitter: Windows Hex test") {
  HostLayout layout = HostLayout::ansi;
  layout.unicodeMode = UnicodeMode::WINDOWS_HEX;
  HostLayouts::SetActiveLayout(layout);
  StenoKeyCodeEmitter emitter;

  const StenoKeyCode codes[] = {
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

  HostLayouts::SetActiveLayout(HostLayout::ansi);
}
TEST_END

#endif

//---------------------------------------------------------------------------

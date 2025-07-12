//---------------------------------------------------------------------------

#include "steno_key_code_emitter.h"
#include "host_layout.h"
#include "key.h"
#include "steno_key_code.h"
#include "steno_key_code_emitter_context.h"
#include "unicode_script.h"

//---------------------------------------------------------------------------

constexpr KeyCode::Value StenoKeyCodeEmitter::EmitterContext::MASK_KEY_CODES[] =
    {
        KeyCode::L_CTRL, KeyCode::L_SHIFT, KeyCode::L_ALT, KeyCode::L_META,
        KeyCode::R_CTRL, KeyCode::R_SHIFT, KeyCode::R_ALT, KeyCode::R_META,
};

//---------------------------------------------------------------------------

StenoKeyCodeEmitter::EmitterContext::EmitterContext()
    : hostLayout(HostLayouts::GetActiveLayout()) {}

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

  if (valueLength > 0) {
    UnicodeScript::instance.SetContext(&context);
    UnicodeScript::instance.ExecuteBeginScript();

    for (size_t i = 0; i < valueLength; ++i) {
      context.ProcessStenoKeyCode(value[i]);
    }
    UnicodeScript::instance.ExecuteEndScript();
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
  const HostLayoutEntry *sequence = hostLayout.GetSequenceForUnicode(unicode);
  if (sequence != nullptr) {
    return EmitSequence(*sequence);
  }

  UnicodeScript::instance.ExecuteEmitScript(unicode);
}

void StenoKeyCodeEmitter::EmitterContext::EmitSequence(
    const HostLayoutEntry &sequence) {
  for (size_t i = 0; i < sequence.length; ++i) {
    EmitKeyCode(sequence.keyCodes[i]);
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
  while (modifiers != 0) {
    ReleaseKey(MASK_KEY_CODES[__builtin_ctzl(modifiers) - MODIFIER_BIT_SHIFT]);

    // Zero the lowest bit.
    modifiers &= modifiers - 1;
  }
}

void StenoKeyCodeEmitter::EmitterContext::ReleaseModifiers() {
  ReleaseModifiers(modifiers);
  modifiers = 0;
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

#endif

//---------------------------------------------------------------------------

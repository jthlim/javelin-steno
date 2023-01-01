//---------------------------------------------------------------------------

#include "script.h"
#include "console.h"
#include "key_code.h"
#include "script_byte_code.h"
#include <assert.h>

//---------------------------------------------------------------------------

class Script::Executor {
public:
  Executor(Script &script) : script(script) {}

  void Run(size_t offset);

private:
  Script &script;
  int locals[4];
  int params[8];
};

//---------------------------------------------------------------------------

Script::Script(const uint8_t *byteCode) : byteCode(byteCode) {
  keyState.ClearAll();
}

void Script::Push(int value) {
  assert(stackTop < stack + MAX_STACK_SIZE);
  *stackTop++ = value;
}

int Script::Pop() {
  assert(stackTop > stack);
  return *--stackTop;
}

void Script::UnaryOp(int (*op)(int)) {
  assert(stackTop > stack);
  stackTop[-1] = op(stackTop[-1]);
}

void Script::BinaryOp(int (*op)(int, int)) {
  assert(stackTop >= stack + 2);
  --stackTop;
  stackTop[-1] = op(stackTop[-1], stackTop[0]);
}

void Script::ExecuteScriptIndex(size_t index) {
  const StenoScriptByteCodeData *data =
      (const StenoScriptByteCodeData *)byteCode;
  Executor executor(*this);
  executor.Run(data->offsets[index]);
}

bool Script::CheckButtonState(const uint8_t *text) const {
  size_t buttonIndex = 0;
  while (*text) {
    switch (*text) {
    case '0':
      // Must be 0.
      if (buttonState.IsSet(buttonIndex)) {
        return false;
      }
      break;

    case ' ':
      // Anything is fine.. skip.
      break;

    default:
      // Must be 1.
      if (!buttonState.IsSet(buttonIndex)) {
        return false;
      }
      break;
    }
    ++buttonIndex;
    ++text;
  }
  return true;
}

__attribute__((weak)) void Script::OnStenoKeyPressed() {}
__attribute__((weak)) void Script::OnStenoKeyReleased() {}
__attribute__((weak)) void Script::OnStenoStateCancelled() {}
__attribute__((weak)) void Script::SendText(const uint8_t *text) const {}

//---------------------------------------------------------------------------

void Script::Executor::Run(size_t offset) {
  using BC = StenoScriptByteCode;
  using OP = StenoScriptOperator;
  using SF = StenoScriptFunction;

  const uint8_t *p = script.byteCode + offset;

  for (;;) {
    uint8_t c = *p++;
    switch (c) {
    case BC::PUSH_CONSTANT_START... BC::PUSH_CONSTANT_END:
      script.Push(c);
      continue;
    case BC::OPERATOR_START... BC::OPERATOR_END: {
      StenoScriptOperator op = StenoScriptOperator(c - BC::OPERATOR_START);
      switch (op) {
      case OP::NOT:
        script.UnaryOp([](int a) -> int { return !a; });
        break;
      case OP::NEGATIVE:
        script.UnaryOp([](int a) -> int { return -a; });
        break;
      case OP::MULTIPLY:
        script.Push(script.Pop() * script.Pop());
        break;
      case OP::QUOTIENT:
        script.BinaryOp([](int a, int b) -> int { return a / b; });
        break;
      case OP::REMAINDER:
        script.BinaryOp([](int a, int b) -> int { return a % b; });
        break;
      case OP::ADD:
        script.BinaryOp([](int a, int b) -> int { return a + b; });
        break;
      case OP::SUBTRACT:
        script.BinaryOp([](int a, int b) -> int { return a - b; });
        break;
      case OP::EQUALS:
        script.BinaryOp([](int a, int b) -> int { return a == b; });
        break;
      case OP::NOT_EQUALS:
        script.BinaryOp([](int a, int b) -> int { return a != b; });
        break;
      case OP::LESS_THAN:
        script.BinaryOp([](int a, int b) -> int { return a < b; });
        break;
      case OP::GREATER_THAN:
        script.BinaryOp([](int a, int b) -> int { return a > b; });
        break;
      case OP::LESS_THAN_OR_EQUAL_TO:
        script.BinaryOp([](int a, int b) -> int { return a <= b; });
        break;
      case OP::GREATER_THAN_OR_EQUAL_TO:
        script.BinaryOp([](int a, int b) -> int { return a >= b; });
        break;
      case OP::BITWISE_AND:
        script.BinaryOp([](int a, int b) -> int { return a & b; });
        break;
      case OP::BITWISE_OR:
        script.BinaryOp([](int a, int b) -> int { return a | b; });
        break;
      case OP::BITWISE_XOR:
        script.BinaryOp([](int a, int b) -> int { return a ^ b; });
        break;
      case OP::AND:
        script.BinaryOp([](int a, int b) -> int { return a && b; });
        break;
      case OP::OR:
        script.BinaryOp([](int a, int b) -> int { return a || b; });
        break;
      }
      continue;
    }
    case BC::JUMP_SHORT_START... BC::JUMP_SHORT_END: {
      int offset = c + 1 - BC::JUMP_SHORT_START;
      p += offset;
      continue;
    }
    case BC::JUMP_IF_ZERO_SHORT_START... BC::JUMP_IF_ZERO_SHORT_END:
      if (!script.Pop()) {
        int offset = c + 1 - BC::JUMP_IF_ZERO_SHORT_START;
        p += offset;
      }
      continue;
    case BC::JUMP_IF_NOT_ZERO_SHORT_START... BC::JUMP_IF_NOT_ZERO_SHORT_END:
      if (script.Pop()) {
        int offset = c + 1 - BC::JUMP_IF_NOT_ZERO_SHORT_START;
        p += offset;
      }
      continue;
    case BC::PUSH_BYTES_1U: {
      int value = *p++;
      if (value < 64) {
        value -= 64;
      }
      script.Push(value);
      continue;
    }
    case BC::PUSH_BYTES_2S: {
      int value = p[0] + (p[1] << 8);
      value <<= 16;
      value >>= 16;
      p += 2;
      script.Push(value);
      continue;
    }
    case BC::PUSH_BYTES_3S: {
      int value = p[0] + (p[1] << 8) + (p[2] << 16);
      value <<= 8;
      value >>= 8;
      p += 3;
      script.Push(value);
      continue;
    }
    case BC::PUSH_BYTES_4: {
      int value = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
      p += 4;
      script.Push(value);
      continue;
    }
    case BC::RETURN:
      return;
    case BC::CALL: {
      size_t offset = p[0] + 256 * p[1];
      p += 2;
      Executor localExecutor(script);
      localExecutor.Run(offset);
      continue;
    }
    case BC::JUMP_LONG:
      p = script.byteCode + p[0] + 256 * p[1];
      continue;
    case BC::JUMP_IF_ZERO_LONG: {
      const uint8_t *target = script.byteCode + p[0] + 256 * p[1];
      p += 2;
      if (!script.Pop()) {
        p = target;
      }
      continue;
    }
    case BC::JUMP_IF_NOT_ZERO_LONG: {
      const uint8_t *target = script.byteCode + p[0] + 256 * p[1];
      p += 2;
      if (script.Pop()) {
        p = target;
      }
      continue;
    }
    case BC::POP:
      script.Pop();
      continue;
    case BC::PARAM_LOAD_START... BC::PARAM_LOAD_END:
      script.Push(params[c - BC::PARAM_LOAD_START]);
      continue;
    case BC::PARAM_STORE_COUNT_START... BC::PARAM_STORE_COUNT_END: {
      int paramIndex = c - BC::PARAM_STORE_COUNT_START;
      while (paramIndex >= 0) {
        params[paramIndex] = script.Pop();
        --paramIndex;
      }
      continue;
    }
    case BC::LOCAL_LOAD_START... BC::LOCAL_LOAD_END:
      script.Push(locals[c - BC::LOCAL_LOAD_START]);
      continue;
    case BC::LOCAL_STORE_START... BC::LOCAL_STORE_END:
      locals[c - BC::LOCAL_STORE_START] = script.Pop();
      continue;
    case BC::GLOBAL_LOAD_START... BC::GLOBAL_LOAD_END:
      script.Push(script.globals[c - BC::GLOBAL_LOAD_START]);
      continue;
    case BC::GLOBAL_STORE_START... BC::GLOBAL_STORE_END:
      script.globals[c - BC::GLOBAL_STORE_START] = script.Pop();
      continue;
    case BC::CALL_FUNCTION_START... BC::CALL_FUNCTION_END: {
      StenoScriptFunction function =
          StenoScriptFunction(c - BC::CALL_FUNCTION_START);

      switch (function) {
      case SF::PRESS_SCAN_CODE: {
        uint32_t key = script.Pop();
        if (key < 256 && !script.keyState.IsSet(key)) {
          script.keyState.Set(key);
          Key::Press(key);
        }
        break;
      }
      case SF::RELEASE_SCAN_CODE: {
        uint32_t key = script.Pop();
        if (key < 256 && script.keyState.IsSet(key)) {
          script.keyState.Clear(key);
          Key::Release(key);
        }
        break;
      }
      case SF::TAP_SCAN_CODE: {
        uint32_t key = script.Pop();
        if (key < 256) {
          if (!script.keyState.IsSet(key)) {
            script.keyState.Clear(key);
            Key::Press(key);
          }
          Key::Release(key);
        }
        break;
      }
      case SF::IS_SCAN_CODE_PRESSED: {
        uint32_t key = script.Pop();
        int isPressed = 0;
        if (key < 256) {
          isPressed = script.keyState.IsSet(key);
        }
        script.Push(isPressed);
        break;
      }
      case SF::PRESS_STENO_KEY: {
        uint32_t stenoKey = script.Pop();
        if (stenoKey < (int)StenoKey::COUNT) {
          script.stenoState |= (1ULL << stenoKey);
        }
        script.OnStenoKeyPressed();
        break;
      }
      case SF::RELEASE_STENO_KEY: {
        uint32_t stenoKey = script.Pop();
        if (stenoKey < (int)StenoKey::COUNT) {
          script.stenoState &= ~StenoKeyState(1ULL << stenoKey);
        }
        script.OnStenoKeyReleased();
        break;
      }
      case SF::IS_STENO_KEY_PRESSED: {
        uint32_t stenoKey = script.Pop();
        int isPressed = 0;
        if (stenoKey < (int)StenoKey::COUNT) {
          isPressed = (script.stenoState & (1ULL << stenoKey)).IsNotEmpty();
        }
        script.Push(isPressed);
        break;
      }
      case SF::RELEASE_ALL:
        for (size_t keyIndex : script.keyState) {
          Key::Release(keyIndex);
        }
        script.keyState.ClearAll();
        script.stenoState = 0;
        script.OnStenoStateCancelled();
        break;
      case SF::IS_BUTTON_PRESSED: {
        uint32_t buttonIndex = script.Pop();
        int isPressed = 0;
        if (buttonIndex < 256) {
          isPressed = script.buttonState.IsSet(buttonIndex);
        }
        script.Push(isPressed);
        break;
      }
      case SF::PRESS_ALL:
        for (size_t buttonIndex : script.buttonState) {
          script.HandlePress(buttonIndex);
        }
        break;
      case SF::SEND_TEXT: {
        int offset = script.Pop();
        const uint8_t *text = script.byteCode + offset;
        script.SendText(text);
        break;
      }
      case SF::CONSOLE: {
        int offset = script.Pop();
        const char *text = (const char *)script.byteCode + offset;
        Console::RunCommand(text);
        break;
      }
      case SF::CHECK_BUTTON_STATE: {
        int offset = script.Pop();
        const uint8_t *text = script.byteCode + offset;
        script.Push(script.CheckButtonState(text));
        break;
      }
      }
      continue;
    }
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

// Created from script:
//
// const SC_A = 4;
// const SC_B = 5;
// const SK_S1 = 0;
//
// var layer = 0;
//
// func onPress0() {
//   if (layer == 1) {
//     pressScanCode(SC_A);
//   } else {
//     pressStenoKey(SK_S1);
//   }
// }
//
// func onRelease0() {
//   if (layer == 1) {
//     releaseScanCode(SC_A);
//   } else {
//     releaseStenoKey(SK_S1);
//   }
// }
//
// // Mode switch key.
// func onPress1() {
//   releaseAll();
//   layer = !layer;
// }
//
// func onRelease1() {
// }

const uint8_t TEST_BYTE_CODE[] = {
    0x4a, 0x53, 0x53, 0x30, 0x28, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x22,
    0x00, 0x27, 0x00, 0xe8, 0x01, 0x47, 0x82, 0x04, 0xf0, 0x61, 0x00,
    0xf4, 0xc4, 0xe8, 0x01, 0x47, 0x82, 0x04, 0xf1, 0x61, 0x00, 0xf5,
    0xc4, 0xf7, 0xe8, 0x40, 0xec, 0xc4, 0xc4, 0x00, 0xec, 0xc4,
};

class ScriptTestHelper {
public:
  static void TestGlobalInitializer() {
    Script script(TEST_BYTE_CODE);
    script.globals[0] = 1;
    script.globals[1] = 2;
    script.globals[2] = 3;
    script.globals[3] = 4;
    script.ExecuteInitScript();
    assert(script.globals[0] == 0);
  }

  static void TestPress0AndRelease0() {
    Script script(TEST_BYTE_CODE);
    script.ExecuteInitScript();

    // Verify S1 is not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());

    script.HandlePress(0);

    // Verify S1 is pressed.
    assert((script.stenoState & StenoKeyState(1)).IsNotEmpty());

    script.HandleRelease(0);

    // Verify S1 is release.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
  }

  static void TestPress1ThenPress0() {
    Script script(TEST_BYTE_CODE);
    script.ExecuteInitScript();

    // Verify S1 and A are not presed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
    assert(script.keyState.IsSet(KeyCode::A) == false);

    script.HandlePress(1);
    script.HandlePress(0);

    // Verify S1 is not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());

    // Verify A is pressed.
    assert(script.keyState.IsSet(KeyCode::A));

    script.HandleRelease(0);

    // Verify S1 and A are not presed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
    assert(script.keyState.IsSet(KeyCode::A) == false);
  }
};

TEST_BEGIN("Script: Initializes globals correctly") {
  ScriptTestHelper::TestGlobalInitializer();
}
TEST_END

TEST_BEGIN("Script: Press0 and Release0 causes S1 to toggle") {
  ScriptTestHelper::TestPress0AndRelease0();
}
TEST_END

TEST_BEGIN("Script: Press1 then Press0 causes A to be pressed") {
  ScriptTestHelper::TestPress1ThenPress0();
}
TEST_END

//---------------------------------------------------------------------------

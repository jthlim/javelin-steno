//---------------------------------------------------------------------------

#include "script.h"
#include "clock.h"
#include "console.h"
#include "display.h"
#include "gpio.h"
#include "key.h"
#include "random.h"
#include "rgb.h"
#include "script_byte_code.h"
#include "usb_status.h"
#include <assert.h>

//---------------------------------------------------------------------------

class Script::ExecutionContext {
public:
  void Run(Script &script, size_t offset);

private:
  intptr_t locals[4];
  intptr_t params[8];
};

//---------------------------------------------------------------------------

Script::Script(const uint8_t *byteCode) : byteCode(byteCode) {
  keyState.ClearAll();
}

void Script::Push(intptr_t value) {
  assert(stackTop < stack + MAX_STACK_SIZE);
  *stackTop++ = value;
}

intptr_t Script::Pop() {
  assert(stackTop > stack);
  return *--stackTop;
}

const intptr_t *Script::Pop(size_t count) {
  assert(stackTop >= stack + count);
  stackTop -= count;
  return stackTop;
}

void Script::UnaryOp(intptr_t (*op)(intptr_t)) {
  assert(stackTop > stack);
  stackTop[-1] = op(stackTop[-1]);
}

void Script::BinaryOp(intptr_t (*op)(intptr_t, intptr_t)) {
  assert(stackTop >= stack + 2);
  --stackTop;
  stackTop[-1] = op(stackTop[-1], stackTop[0]);
}

void Script::ExecuteScriptIndex(size_t index) {
  const StenoScriptByteCodeData *data =
      (const StenoScriptByteCodeData *)byteCode;
  ExecutionContext context;
  context.Run(*this, data->offsets[index]);
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
__attribute__((weak)) bool Script::ProcessScanCode(int scanCode,
                                                   ScanCodeAction action) {
  return false;
}
__attribute__((weak)) void Script::SendText(const uint8_t *text) const {}

//---------------------------------------------------------------------------

void Script::ExecutionContext::Run(Script &script, size_t offset) {
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
        script.UnaryOp([](intptr_t a) -> intptr_t { return !a; });
        break;
      case OP::NEGATIVE:
        script.UnaryOp([](intptr_t a) -> intptr_t { return -a; });
        break;
      case OP::MULTIPLY:
        script.Push(script.Pop() * script.Pop());
        break;
      case OP::QUOTIENT:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a / b; });
        break;
      case OP::REMAINDER:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a % b; });
        break;
      case OP::ADD:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a + b; });
        break;
      case OP::SUBTRACT:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a - b; });
        break;
      case OP::EQUALS:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a == b; });
        break;
      case OP::NOT_EQUALS:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a != b; });
        break;
      case OP::LESS_THAN:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a < b; });
        break;
      case OP::GREATER_THAN:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a > b; });
        break;
      case OP::LESS_THAN_OR_EQUAL_TO:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a <= b; });
        break;
      case OP::GREATER_THAN_OR_EQUAL_TO:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a >= b; });
        break;
      case OP::BITWISE_AND:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a & b; });
        break;
      case OP::BITWISE_OR:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a | b; });
        break;
      case OP::BITWISE_XOR:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a ^ b; });
        break;
      case OP::AND:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a && b; });
        break;
      case OP::OR:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a || b; });
        break;
      case OP::SHIFT_LEFT:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a << b; });
        break;
      case OP::ARITHMETIC_SHIFT_RIGHT:
        script.BinaryOp(
            [](intptr_t a, intptr_t b) -> intptr_t { return a >> b; });
        break;
      case OP::LOGICAL_SHIFT_RIGHT:
        script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t {
          return uintptr_t(a) >> b;
        });
        break;
      case OP::BYTE_LOOKUP: {
        intptr_t index = script.Pop();
        intptr_t offset = script.Pop();
        const uint8_t *data = script.byteCode + offset;
        script.Push(data[index]);
        break;
      }
      case OP::WORD_LOOKUP: {
        intptr_t index = script.Pop();
        intptr_t offset = script.Pop();
        const int32_t *data = (const int32_t *)(script.byteCode + offset);
        script.Push(data[index]);
        break;
      }
      case OP::INCREMENT:
        script.UnaryOp([](intptr_t a) -> intptr_t { return a + 1; });
        break;
      case OP::DECREMENT:
        script.UnaryOp([](intptr_t a) -> intptr_t { return a - 1; });
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
      ExecutionContext localContext;
      localContext.Run(script, offset);
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
    case BC::GLOBAL_LOAD: {
      int globalIndex = *p++;
      script.Push(script.globals[globalIndex]);
      break;
    }
    case BC::GLOBAL_STORE: {
      int globalIndex = *p++;
      script.globals[globalIndex] = script.Pop();
      break;
    }
    case BC::EXTENDED_CALL_FUNCTION: {
      StenoExtendedScriptFunction function = StenoExtendedScriptFunction(*p++);

      switch (function) {
      case StenoExtendedScriptFunction::GET_LED_STATUS: {
        int index = (int)script.Pop();
        script.Push(Key::GetLedStatus(index));
        break;
      }
      case StenoExtendedScriptFunction::SET_GPIO_PIN: {
        int enable = script.Pop() != 0;
        int pin = (int)script.Pop();
        Gpio::SetPin(pin, enable);
        break;
      }
      case StenoExtendedScriptFunction::CLEAR_DISPLAY: {
        int displayId = (int)script.Pop();
        Display::Clear(displayId);
        break;
      }
      case StenoExtendedScriptFunction::SET_AUTO_DRAW: {
        int autoDrawId = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::SetAutoDraw(displayId, autoDrawId);
        break;
      }
      case StenoExtendedScriptFunction::SET_SCREEN_ON: {
        bool on = script.Pop() != 0;
        int displayId = (int)script.Pop();
        Display::SetScreenOn(displayId, on);
        break;
      }
      case StenoExtendedScriptFunction::SET_SCREEN_CONTRAST: {
        int contrast = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::SetContrast(displayId, contrast);
        break;
      }
      case StenoExtendedScriptFunction::DRAW_PIXEL: {
        int y = (int)script.Pop();
        int x = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawPixel(displayId, x, y);
        break;
      }
      case StenoExtendedScriptFunction::DRAW_LINE: {
        int y2 = (int)script.Pop();
        int x2 = (int)script.Pop();
        int y1 = (int)script.Pop();
        int x1 = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawLine(displayId, x1, y1, x2, y2);
        break;
      }
      case StenoExtendedScriptFunction::DRAW_IMAGE: {
        intptr_t offset = script.Pop();
        const uint8_t *data = (const uint8_t *)script.byteCode + offset;
        int y = (int)script.Pop();
        int x = (int)script.Pop();
        int displayId = (int)script.Pop();
        int width = *data++;
        int height = *data++;
        Display::DrawImage(displayId, x, y, width, height, data);
        break;
      }
      case StenoExtendedScriptFunction::DRAW_TEXT: {
        intptr_t offset = script.Pop();
        const char *text = (const char *)script.byteCode + offset;
        TextAlignment alignment = (TextAlignment)script.Pop();
        FontId fontId = (FontId)script.Pop();
        int y = (int)script.Pop();
        int x = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawText(displayId, x, y, fontId, alignment, text);
        break;
      }
      case StenoExtendedScriptFunction::SET_DRAW_COLOR: {
        int color = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::SetDrawColor(displayId, color);
        break;
      }
      case StenoExtendedScriptFunction::DRAW_RECT: {
        int bottom = (int)script.Pop();
        int right = (int)script.Pop();
        int top = (int)script.Pop();
        int left = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawRect(displayId, left, top, right, bottom);
        break;
      }
      case StenoExtendedScriptFunction::SET_HSV: {
        int v = (int)script.Pop();
        int s = (int)script.Pop();
        int h = (int)script.Pop();
        int id = (int)script.Pop();
        Rgb::SetHsv(id, h, s, v);
        break;
      }
      case StenoExtendedScriptFunction::RAND:
        script.Push(Random::GenerateUint32());
        break;
      case StenoExtendedScriptFunction::IS_USB_MOUNTED:
        script.Push(UsbStatus::IsMounted());
        break;
      case StenoExtendedScriptFunction::IS_USB_SUSPENDED:
        script.Push(UsbStatus::IsSuspended());
        break;
      }
      break;
    }
    case BC::GLOBAL_LOAD_INDEX: {
      intptr_t globalBaseIndex = *p++;
      intptr_t index = script.Pop();
      script.Push(script.globals[globalBaseIndex + index]);
      break;
    }
    case BC::GLOBAL_STORE_INDEX: {
      int globalBaseIndex = *p++;
      intptr_t value = script.Pop();
      intptr_t index = script.Pop();
      script.globals[globalBaseIndex + index] = value;
      break;
    }
    case BC::PARAM_STORE: {
      int paramIndex = *p++;
      params[paramIndex] = script.Pop();
      break;
    }
    case BC::PARAM_LOAD_START... BC::PARAM_LOAD_END:
      script.Push(params[c - BC::PARAM_LOAD_START]);
      continue;
    case BC::PARAM_STORE_COUNT_START + 7:
      params[7] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START + 6:
      params[6] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START + 5:
      params[5] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START + 4:
      params[4] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START + 3:
      params[3] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START + 2:
      params[2] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START + 1:
      params[1] = script.Pop();
      [[fallthrough]];
    case BC::PARAM_STORE_COUNT_START:
      params[0] = script.Pop();
      continue;
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
        uint32_t key = (uint32_t)script.Pop();
        if (key < 256 && !script.keyState.IsSet(key)) {
          if (!script.ProcessScanCode(key, ScanCodeAction::PRESS)) {
            script.keyState.Set(key);
            Key::Press(key);
          }
        }
        break;
      }
      case SF::RELEASE_SCAN_CODE: {
        uint32_t key = (uint32_t)script.Pop();
        if (key < 256 && script.keyState.IsSet(key)) {
          if (!script.ProcessScanCode(key, ScanCodeAction::RELEASE)) {
            script.keyState.Clear(key);
            Key::Release(key);
          }
        }
        break;
      }
      case SF::TAP_SCAN_CODE: {
        uint32_t key = (uint32_t)script.Pop();
        if (key < 256) {
          if (!script.ProcessScanCode(key, ScanCodeAction::TAP)) {
            if (!script.keyState.IsSet(key)) {
              script.keyState.Clear(key);
              Key::Press(key);
            }
            Key::Release(key);
          }
        }
        break;
      }
      case SF::IS_SCAN_CODE_PRESSED: {
        uint32_t key = (uint32_t)script.Pop();
        int isPressed = 0;
        if (key < 256) {
          isPressed = script.keyState.IsSet(key);
        }
        script.Push(isPressed);
        break;
      }
      case SF::PRESS_STENO_KEY: {
        uint32_t stenoKey = (uint32_t)script.Pop();
        if (stenoKey < (uint32_t)StenoKey::COUNT) {
          script.stenoState |= (1ULL << stenoKey);
          script.OnStenoKeyPressed();
        }
        break;
      }
      case SF::RELEASE_STENO_KEY: {
        uint32_t stenoKey = (uint32_t)script.Pop();
        if (stenoKey < (uint32_t)StenoKey::COUNT) {
          script.stenoState &= ~StenoKeyState(1ULL << stenoKey);
          script.OnStenoKeyReleased();
        }
        break;
      }
      case SF::IS_STENO_KEY_PRESSED: {
        uint32_t stenoKey = (uint32_t)script.Pop();
        int isPressed = 0;
        if (stenoKey < (uint32_t)StenoKey::COUNT) {
          isPressed = (script.stenoState & (1ULL << stenoKey)).IsNotEmpty();
        }
        script.Push(isPressed);
        break;
      }
      case SF::RELEASE_ALL:
        for (size_t keyIndex : script.keyState) {
          Key::Release(uint32_t(keyIndex));
        }
        script.keyState.ClearAll();
        script.stenoState = 0;
        script.OnStenoStateCancelled();
        break;
      case SF::IS_BUTTON_PRESSED: {
        uint32_t buttonIndex = (uint32_t)script.Pop();
        int isPressed = 0;
        if (buttonIndex < 256) {
          isPressed = script.buttonState.IsSet(buttonIndex);
        }
        script.Push(isPressed);
        break;
      }
      case SF::PRESS_ALL:
        script.inPressAllCount++;
        for (size_t buttonIndex : script.buttonState) {
          script.HandlePress(buttonIndex);
        }
        script.inPressAllCount--;
        break;
      case SF::SEND_TEXT: {
        intptr_t offset = script.Pop();
        const uint8_t *text = script.byteCode + offset;
        script.SendText(text);
        break;
      }
      case SF::CONSOLE: {
        intptr_t offset = script.Pop();
        const char *text = (const char *)script.byteCode + offset;
        script.consoleWriter.Reset();

        const uint8_t *result;
        if (Console::RunCommand(text, script.consoleWriter)) {
          script.consoleWriter.AddTrailingNull();
          result = script.consoleWriter.buffer;
        } else {
          result = (const uint8_t *)"Invalid console command";
        }

        script.Push(int(result - script.byteCode));
        break;
      }
      case SF::CHECK_BUTTON_STATE: {
        intptr_t offset = script.Pop();
        const uint8_t *text = script.byteCode + offset;
        script.Push(script.CheckButtonState(text));
        break;
      }
      case SF::IS_IN_PRESS_ALL:
        script.Push(script.inPressAllCount);
        break;
      case SF::SET_RGB: {
        int b = script.Pop() & 0xff;
        int g = script.Pop() & 0xff;
        int r = script.Pop() & 0xff;
        int id = (int)script.Pop();
        Rgb::SetRgb(id, r, g, b);
        break;
      }
      case SF::GET_TIME:
        script.Push(Clock::GetCurrentTime());
        break;
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
    0x4a, 0x53, 0x53, 0x30, 0x2a, 0x00, 0x2d, 0x00, 0x10, 0x00, 0x1a, 0x00,
    0x24, 0x00, 0x29, 0x00, 0xe8, 0x01, 0x47, 0x82, 0x04, 0xf0, 0xc4, 0x00,
    0xf4, 0xc4, 0xe8, 0x01, 0x47, 0x82, 0x04, 0xf1, 0xc4, 0x00, 0xf5, 0xc4,
    0xf7, 0xe8, 0x40, 0xec, 0xc4, 0xc4, 0x00, 0xec, 0xc4, 0xc4,
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

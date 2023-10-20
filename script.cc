//---------------------------------------------------------------------------

#include "script.h"
#include "clock.h"
#include "console.h"
#include "hal/ble.h"
#include "hal/connection.h"
#include "hal/display.h"
#include "hal/gpio.h"
#include "hal/power.h"
#include "hal/rgb.h"
#include "hal/usb_status.h"
#include "key.h"
#include "keyboard_led_status.h"
#include "malloc_allocate.h"
#include "random.h"
#include "script_byte_code.h"
#include "split/split_usb_status.h"
#include "str.h"
#include "timer_manager.h"
#include <assert.h>

//---------------------------------------------------------------------------

class Script::ExecutionContext {
public:
  void Run(Script &script, size_t offset);

private:
  intptr_t *base; // The stack point that the code needs to pop to to exit.
  intptr_t *locals;
  intptr_t *frame; // The working area for the current function
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

void Script::ExecuteScript(size_t offset, uint32_t scriptTime) {
  if (offset == 0) {
    return;
  }

  this->scriptTime = scriptTime;

#if DEBUG
  intptr_t *start = stackTop;
#endif

  ExecutionContext context;
  context.Run(*this, offset);

#if DEBUG
  assert(stackTop == start);
#endif
}

void Script::ExecuteScriptId(ScriptId scriptId, uint32_t scriptTime) {
  size_t offset = scriptOffsets[(size_t)scriptId];
  if (offset != 0) {
    ExecuteScript(offset, scriptTime);
  }
}

inline bool Script::IsScriptEmpty(size_t offset) const {
  return offset == 0 || byteCode[offset] == StenoScriptByteCode::RETURN;
}

bool Script::IsScriptIndexEmpty(size_t index) const {
  const StenoScriptByteCodeData *data =
      (const StenoScriptByteCodeData *)byteCode;
  return IsScriptEmpty(data->offsets[index]);
}

void Script::ExecuteScriptIndex(size_t index, uint32_t scriptTime) {
  const StenoScriptByteCodeData *data =
      (const StenoScriptByteCodeData *)byteCode;
  ExecuteScript(data->offsets[index], scriptTime);
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

//---------------------------------------------------------------------------

void Script::PrintInfo() const {
  Console::Printf("Script\n");

  Console::Printf("  Callbacks: [");
  bool firstTime = true;
  for (size_t i = 0; i < (size_t)ScriptId::COUNT; ++i) {
    if (scriptOffsets[i] != 0) {
      if (firstTime) {
        firstTime = false;
        Console::Printf("%zu", i);
      } else {
        Console::Printf(", %zu", i);
      }
    }
  }
  Console::Printf("]\n");
}

//---------------------------------------------------------------------------

struct Script::ScriptTimerContext : public JavelinMallocAllocate {
  Script *script;
  size_t offset;

  void Run(intptr_t id) const {
    script->stack[0] = id;
    script->stackTop = &script->stack[1];

    ExecutionContext executionContext;
    executionContext.Run(*script, offset);

    script->stackTop = script->stack;
  }
};

void Script::TimerHandler(intptr_t id, void *context) {
  ScriptTimerContext *scriptContext = (ScriptTimerContext *)context;
  scriptContext->Run(id);
}

void Script::StopTimer(intptr_t timerId) {
  ScriptTimerContext *oldContext =
      (ScriptTimerContext *)TimerManager::instance.StopTimer(timerId,
                                                             scriptTime);
  delete oldContext;
}

void Script::StartTimer(intptr_t timerId, uint32_t interval, bool isRepeating,
                        size_t offset) {
  ScriptTimerContext *context = new ScriptTimerContext;
  context->script = this;
  context->offset = offset;

  ScriptTimerContext *oldContext =
      (ScriptTimerContext *)TimerManager::instance.StartTimer(
          timerId, interval, isRepeating, TimerHandler, context, scriptTime);

  delete oldContext;
}

//---------------------------------------------------------------------------

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

  base = script.stackTop;
  frame = script.stackTop;

  const uint8_t *p = script.byteCode + offset;

  for (;;) {
    uint8_t c = *p++;
    switch (c) {
    case BC::PUSH_CONSTANT_START... BC::PUSH_CONSTANT_END:
      script.Push(c);
      continue;
    case BC::PUSH_BYTES_1U: {
      int value = *p++;
      if (value < 0x3c) {
        value -= 0x3c;
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
    case BC::LOAD_GLOBAL_BEGIN... BC::LOAD_GLOBAL_END:
      script.Push(script.globals[c - BC::LOAD_GLOBAL_BEGIN]);
      continue;
    case BC::LOAD_GLOBAL_VALUE: {
      int globalIndex = *p++;
      script.Push(script.globals[globalIndex]);
      continue;
    }
    case BC::LOAD_GLOBAL_INDEX: {
      intptr_t globalBaseIndex = *p++;
      intptr_t index = script.Pop();
      script.Push(script.globals[globalBaseIndex + index]);
      continue;
    }
    case BC::STORE_GLOBAL_BEGIN... BC::STORE_GLOBAL_END:
      script.globals[c - BC::STORE_GLOBAL_BEGIN] = script.Pop();
      continue;
    case BC::STORE_GLOBAL_VALUE: {
      int globalIndex = *p++;
      script.globals[globalIndex] = script.Pop();
      continue;
    }
    case BC::STORE_GLOBAL_INDEX: {
      int globalBaseIndex = *p++;
      intptr_t value = script.Pop();
      intptr_t index = script.Pop();
      script.globals[globalBaseIndex + index] = value;
      continue;
    }
    case BC::LOAD_LOCAL_BEGIN... BC::LOAD_LOCAL_END:
      script.Push(locals[c - BC::LOAD_LOCAL_BEGIN]);
      continue;
    case BC::LOAD_LOCAL_VALUE: {
      int localIndex = *p++;
      script.Push(locals[localIndex]);
      continue;
    }
    case BC::LOAD_LOCAL_INDEX: {
      int localBaseIndex = *p++;
      intptr_t index = script.Pop();
      script.Push(locals[localBaseIndex + index]);
      continue;
    }
    case BC::STORE_LOCAL_BEGIN... BC::STORE_LOCAL_END:
      locals[c - BC::STORE_LOCAL_BEGIN] = script.Pop();
      continue;
    case BC::STORE_LOCAL_VALUE: {
      int localIndex = *p++;
      locals[localIndex] = script.Pop();
      continue;
    }
    case BC::STORE_LOCAL_INDEX: {
      int localBaseIndex = *p++;
      intptr_t value = script.Pop();
      intptr_t index = script.Pop();
      locals[localBaseIndex + index] = value;
      continue;
    }
    case BC::OPERATOR_START + (int)OP::NOT:
      script.UnaryOp([](intptr_t a) -> intptr_t { return !a; });
      continue;
    case BC::OPERATOR_START + (int)OP::NEGATIVE:
      script.UnaryOp([](intptr_t a) -> intptr_t { return -a; });
      continue;
    case BC::OPERATOR_START + (int)OP::MULTIPLY:
      script.Push(script.Pop() * script.Pop());
      continue;
    case BC::OPERATOR_START + (int)OP::QUOTIENT:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a / b; });
      continue;
    case BC::OPERATOR_START + (int)OP::REMAINDER:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a % b; });
      continue;
    case BC::OPERATOR_START + (int)OP::ADD:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a + b; });
      continue;
    case BC::OPERATOR_START + (int)OP::SUBTRACT:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a - b; });
      continue;
    case BC::OPERATOR_START + (int)OP::EQUALS:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a == b; });
      continue;
    case BC::OPERATOR_START + (int)OP::NOT_EQUALS:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a != b; });
      continue;
    case BC::OPERATOR_START + (int)OP::LESS_THAN:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a < b; });
      continue;
    case BC::OPERATOR_START + (int)OP::GREATER_THAN:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a > b; });
      continue;
    case BC::OPERATOR_START + (int)OP::LESS_THAN_OR_EQUAL_TO:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a <= b; });
      continue;
    case BC::OPERATOR_START + (int)OP::GREATER_THAN_OR_EQUAL_TO:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a >= b; });
      continue;
    case BC::OPERATOR_START + (int)OP::BITWISE_AND:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a & b; });
      continue;
    case BC::OPERATOR_START + (int)OP::BITWISE_OR:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a | b; });
      continue;
    case BC::OPERATOR_START + (int)OP::BITWISE_XOR:
      script.BinaryOp([](intptr_t a, intptr_t b) -> intptr_t { return a ^ b; });
      continue;
    case BC::OPERATOR_START + (int)OP::AND:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a && b; });
      continue;
    case BC::OPERATOR_START + (int)OP::OR:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a || b; });
      continue;
    case BC::OPERATOR_START + (int)OP::SHIFT_LEFT:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a << b; });
      continue;
    case BC::OPERATOR_START + (int)OP::ARITHMETIC_SHIFT_RIGHT:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return a >> b; });
      continue;
    case BC::OPERATOR_START + (int)OP::LOGICAL_SHIFT_RIGHT:
      script.BinaryOp(
          [](intptr_t a, intptr_t b) -> intptr_t { return uintptr_t(a) >> b; });
      continue;
    case BC::OPERATOR_START + (int)OP::BYTE_LOOKUP: {
      intptr_t index = script.Pop();
      intptr_t offset = script.Pop();
      const uint8_t *data = script.byteCode + offset;
      script.Push(data[index]);
      continue;
    }
    case BC::OPERATOR_START + (int)OP::WORD_LOOKUP: {
      intptr_t index = script.Pop();
      intptr_t offset = script.Pop();
      const int32_t *data = (const int32_t *)(script.byteCode + offset);
      script.Push(data[index]);
      continue;
    }
    case BC::OPERATOR_START + (int)OP::INCREMENT:
      script.UnaryOp([](intptr_t a) -> intptr_t { return a + 1; });
      continue;
    case BC::OPERATOR_START + (int)OP::DECREMENT:
      script.UnaryOp([](intptr_t a) -> intptr_t { return a - 1; });
      continue;
    case BC::CALL_INTERNAL: {
      StenoScriptFunction function = StenoScriptFunction(*p++);

      switch (function) {
      case SF::PRESS_SCAN_CODE: {
        uint32_t key = (uint32_t)script.Pop();
        if (key < 256 && !script.keyState.IsSet(key)) {
          if (!script.ProcessScanCode(key, ScanCodeAction::PRESS)) {
            script.keyState.Set(key);
            Key::Press(key);
          }
        }
        continue;
      }
      case SF::RELEASE_SCAN_CODE: {
        uint32_t key = (uint32_t)script.Pop();
        if (key < 256 && script.keyState.IsSet(key)) {
          if (!script.ProcessScanCode(key, ScanCodeAction::RELEASE)) {
            script.keyState.Clear(key);
            Key::Release(key);
          }
        }
        continue;
      }
      case SF::TAP_SCAN_CODE: {
        uint32_t key = (uint32_t)script.Pop();
        if (key < 256) {
          if (!script.ProcessScanCode(key, ScanCodeAction::TAP)) {
            if (script.keyState.IsSet(key)) {
              script.keyState.Clear(key);
            } else {
              Key::Press(key);
            }
            Key::Release(key);
          }
        }
        continue;
      }
      case SF::IS_SCAN_CODE_PRESSED: {
        uint32_t key = (uint32_t)script.Pop();
        int isPressed = 0;
        if (key < 256) {
          isPressed = script.keyState.IsSet(key);
        }
        script.Push(isPressed);
        continue;
      }
      case SF::PRESS_STENO_KEY: {
        uint32_t stenoKey = (uint32_t)script.Pop();
        if (stenoKey < (uint32_t)StenoKey::COUNT) {
          script.stenoState |= (1ULL << stenoKey);
          script.OnStenoKeyPressed();
        }
        continue;
      }
      case SF::RELEASE_STENO_KEY: {
        uint32_t stenoKey = (uint32_t)script.Pop();
        if (stenoKey < (uint32_t)StenoKey::COUNT) {
          script.stenoState &= ~StenoKeyState(1ULL << stenoKey);
          script.OnStenoKeyReleased();
        }
        continue;
      }
      case SF::IS_STENO_KEY_PRESSED: {
        uint32_t stenoKey = (uint32_t)script.Pop();
        int isPressed = 0;
        if (stenoKey < (uint32_t)StenoKey::COUNT) {
          isPressed = (script.stenoState & (1ULL << stenoKey)).IsNotEmpty();
        }
        script.Push(isPressed);
        continue;
      }
      case SF::RELEASE_ALL:
        for (size_t keyIndex : script.keyState) {
          Key::Release(uint32_t(keyIndex));
        }
        script.keyState.ClearAll();
        script.stenoState = 0;
        script.OnStenoStateCancelled();
        continue;
      case SF::IS_BUTTON_PRESSED: {
        uint32_t buttonIndex = (uint32_t)script.Pop();
        int isPressed = 0;
        if (buttonIndex < 256) {
          isPressed = script.buttonState.IsSet(buttonIndex);
        }
        script.Push(isPressed);
        continue;
      }
      case SF::PRESS_ALL:
        script.inPressAllCount++;
        for (size_t buttonIndex : script.buttonState) {
          script.HandlePress(buttonIndex, script.scriptTime);
        }
        script.inPressAllCount--;
        continue;
      case SF::SEND_TEXT: {
        intptr_t offset = script.Pop();
        const uint8_t *text = script.byteCode + offset;
        script.SendText(text);
        continue;
      }
      case SF::CONSOLE: {
        intptr_t offset = script.Pop();
        const char *command = (const char *)script.byteCode + offset;
        script.RunConsoleCommand(command);
        continue;
      }
      case SF::CHECK_BUTTON_STATE: {
        intptr_t offset = script.Pop();
        const uint8_t *text = script.byteCode + offset;
        script.Push(script.CheckButtonState(text));
        continue;
      }
      case SF::IS_IN_PRESS_ALL:
        script.Push(script.inPressAllCount);
        continue;
      case SF::SET_RGB: {
        int b = script.Pop() & 0xff;
        int g = script.Pop() & 0xff;
        int r = script.Pop() & 0xff;
        int id = (int)script.Pop();
        Rgb::SetRgb(id, r, g, b);
        continue;
      }
      case SF::GET_TIME:
        script.Push(script.scriptTime);
        continue;
      case SF::GET_LED_STATUS: {
        int index = (int)script.Pop();
        script.Push(
            Connection::GetActiveKeyboardLedStatus().GetLedStatus(index));
        continue;
      }
      case SF::SET_GPIO_PIN: {
        int enable = script.Pop() != 0;
        int pin = (int)script.Pop();
        Gpio::SetPin(pin, enable);
        continue;
      }
      case SF::CLEAR_DISPLAY: {
        int displayId = (int)script.Pop();
        Display::Clear(displayId);
        continue;
      }
      case SF::SET_AUTO_DRAW: {
        int autoDrawId = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::SetAutoDraw(displayId, autoDrawId);
        continue;
      }
      case SF::SET_SCREEN_ON: {
        bool on = script.Pop() != 0;
        int displayId = (int)script.Pop();
        Display::SetScreenOn(displayId, on);
        continue;
      }
      case SF::SET_SCREEN_CONTRAST: {
        int contrast = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::SetContrast(displayId, contrast);
        continue;
      }
      case SF::DRAW_PIXEL: {
        int y = (int)script.Pop();
        int x = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawPixel(displayId, x, y);
        continue;
      }
      case SF::DRAW_LINE: {
        int y2 = (int)script.Pop();
        int x2 = (int)script.Pop();
        int y1 = (int)script.Pop();
        int x1 = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawLine(displayId, x1, y1, x2, y2);
        continue;
      }
      case SF::DRAW_IMAGE: {
        intptr_t offset = script.Pop();
        const uint8_t *data = (const uint8_t *)script.byteCode + offset;
        int y = (int)script.Pop();
        int x = (int)script.Pop();
        int displayId = (int)script.Pop();
        int width = *data++;
        int height = *data++;
        Display::DrawImage(displayId, x, y, width, height, data);
        continue;
      }
      case SF::DRAW_TEXT: {
        intptr_t offset = script.Pop();
        const char *text = (const char *)script.byteCode + offset;
        TextAlignment alignment = (TextAlignment)script.Pop();
        FontId fontId = (FontId)script.Pop();
        int y = (int)script.Pop();
        int x = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawText(displayId, x, y, fontId, alignment, text);
        continue;
      }
      case SF::SET_DRAW_COLOR: {
        int color = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::SetDrawColor(displayId, color);
        continue;
      }
      case SF::DRAW_RECT: {
        int bottom = (int)script.Pop();
        int right = (int)script.Pop();
        int top = (int)script.Pop();
        int left = (int)script.Pop();
        int displayId = (int)script.Pop();
        Display::DrawRect(displayId, left, top, right, bottom);
        continue;
      }
      case SF::SET_HSV: {
        int v = (int)script.Pop();
        int s = (int)script.Pop();
        int h = (int)script.Pop();
        int id = (int)script.Pop();
        Rgb::SetHsv(id, h, s, v);
        continue;
      }
      case SF::RAND:
        script.Push(Random::GenerateUint32());
        continue;
      case SF::IS_USB_MOUNTED:
        script.Push(UsbStatus::instance.IsConnected() ||
                    SplitUsbStatus::instance.IsConnected());
        continue;
      case SF::IS_USB_SUSPENDED:
        script.Push(UsbStatus::instance.IsSleeping() ||
                    SplitUsbStatus::instance.IsSleeping());
        continue;
      case SF::GET_PARAMETER: {
        intptr_t offset = script.Pop();
        const char *parameter = (const char *)script.byteCode + offset;
        script.RunGetParameterCommand(parameter);
        continue;
      }
      case SF::IS_CONNECTED: {
        ConnectionId connectionId = (ConnectionId)script.Pop();
        script.Push(Connection::IsConnected(connectionId));
        continue;
      }
      case SF::GET_ACTIVE_CONNECTION:
        script.Push((int)Connection::GetActiveConnection());
        continue;
      case SF::SET_PREFERRED_CONNECTION: {
        ConnectionId third = (ConnectionId)script.Pop();
        ConnectionId second = (ConnectionId)script.Pop();
        ConnectionId first = (ConnectionId)script.Pop();
        Connection::SetPreferredConnection(first, second, third);
        continue;
      }
      case SF::IS_PAIR_CONNECTED: {
        PairConnectionId pairConnectionId = (PairConnectionId)script.Pop();
        script.Push(Connection::IsPairConnected(pairConnectionId));
        continue;
      }
      case SF::START_BLE_PAIRING:
        Ble::StartPairing();
        continue;
      case SF::GET_BLE_PROFILE:
        script.Push(Ble::GetProfile());
        continue;
      case SF::SET_BLE_PROFILE:
        Ble::SetProfile((int)script.Pop());
        continue;
      case SF::IS_HOST_SLEEPING:
        script.Push(Connection::IsHostSleeping());
        continue;
      case SF::IS_MAIN_POWERED:
        script.Push(UsbStatus::instance.IsPowered());
        continue;
      case SF::IS_CHARGING:
        script.Push(Power::IsCharging());
        continue;
      case SF::GET_BATTERY_PERCENTAGE:
        script.Push(Power::GetBatteryPercentage());
        continue;
      case SF::GET_ACTIVE_PAIR_CONNECTION:
        script.Push((int)Connection::GetActivePairConnection());
        continue;
      case SF::SET_BOARD_POWER:
        Power::SetBoardPower(script.Pop() != 0);
        continue;
      case SF::SEND_EVENT: {
        intptr_t offset = script.Pop();
        if (script.scriptEventsEnabled) {
          const char *text = (const char *)script.byteCode + offset;
          Console::WriteScriptEvent(text);
        }
        continue;
      }
      case SF::IS_PAIR_POWERED:
        script.Push(SplitUsbStatus::instance.IsPowered());
        continue;
      case SF::SET_INPUT_HINT:
        script.SetInputHint((int)script.Pop());
        continue;
      case SF::SET_SCRIPT: {
        size_t scriptOffset = script.Pop();
        ScriptId scriptId = (ScriptId)script.Pop();
        script.SetScript(scriptId, scriptOffset);
        continue;
      }
      case SF::IS_BOARD_POWERED:
        script.Push(Power::IsBoardPowered());
        continue;
      case SF::START_TIMER: {
        size_t scriptOffset = script.Pop();
        bool repeating = script.Pop() != 0;
        uint32_t interval = (uint32_t)script.Pop();
        intptr_t id = script.Pop();
        script.StartTimer(id, interval, repeating, scriptOffset);
        continue;
      }
      case SF::STOP_TIMER: {
        intptr_t id = script.Pop();
        script.StopTimer(id);
        continue;
      }
      case SF::IS_TIMER_ACTIVE: {
        intptr_t id = script.Pop();
        script.Push(TimerManager::instance.HasTimer(id));
        continue;
      }
      case SF::IS_BLE_PROFILE_CONNECTED: {
        uint32_t profileId = (uint32_t)script.Pop();
        script.Push(Ble::IsProfileConnected(profileId));
        continue;
      }
      case SF::DISCONNECT_BLE:
        Ble::Disconnect();
        continue;
      case SF::IS_BLE_PROFILE_PAIRED: {
        uint32_t profileId = (uint32_t)script.Pop();
        script.Push(Ble::IsProfilePaired(profileId));
        continue;
      }
      case SF::UNPAIR_BLE:
        Ble::Unpair();
        continue;
      case SF::IS_BLE_PROFILE_SLEEPING: {
        uint32_t profileId = (uint32_t)script.Pop();
        script.Push(Ble::IsProfileSleeping(profileId));
        continue;
      }
      case SF::IS_BLE_ADVERTISING:
        script.Push(Ble::IsAdvertising());
        continue;
      case SF::IS_BLE_SCANNING:
        script.Push(Ble::IsScanning());
        continue;
      case SF::IS_WAITING_FOR_USER_PRESENCE:
        script.Push(IsWaitingForUserPresence());
        continue;
      case SF::REPLY_USER_PRESENCE:
        script.ReplyUserPresence(script.Pop() != 0);
        continue;
      case SF::SET_GPIO_INPUT_PIN: {
        int pull = script.Pop();
        int pin = script.Pop();
        Gpio::SetInputPin(pin, (Gpio::Pull)pull);
        continue;
      }
      case SF::READ_GPIO_PIN: {
        int pin = script.Pop();
        script.Push(Gpio::GetPin(pin));
        continue;
      }
      }
      continue;
    }
    case BC::CALL: {
      size_t offset = p[0] + 256 * p[1];
      p += 2;
      ExecutionContext localContext;
      localContext.Run(script, offset);
      continue;
    }
    case BC::RETURN: {
      intptr_t *p = base;
      if (frame != script.stackTop) {
        *p++ = *frame;
      }
      script.stackTop = p;
      return;
    }
    case BC::POP:
      script.Pop();
      continue;
    case BC::ENTER_FUNCTION: {
      size_t parameterCount = *p++;
      size_t localsCount = *p++;

      locals = script.stackTop - parameterCount;
      if (locals < base) {
        base = locals;
      }

      frame = script.stackTop + localsCount;
      script.stackTop = frame;
      continue;
    }
    case BC::CALL_VALUE: {
      size_t offset = script.Pop();
      ExecutionContext localContext;
      localContext.Run(script, offset);
      continue;
    }
    case BC::JUMP_VALUE:
      p = script.byteCode + script.Pop();
      continue;
    case BC::JUMP_SHORT_BEGIN... BC::JUMP_SHORT_END: {
      int offset = c + 1 - BC::JUMP_SHORT_BEGIN;
      p += offset;
      continue;
    }
    case BC::JUMP_LONG:
      p = script.byteCode + p[0] + 256 * p[1];
      continue;
    case BC::JUMP_IF_ZERO_SHORT_BEGIN... BC::JUMP_IF_ZERO_SHORT_END:
      if (!script.Pop()) {
        int offset = c + 1 - BC::JUMP_IF_ZERO_SHORT_BEGIN;
        p += offset;
      }
      continue;
    case BC::JUMP_IF_ZERO_LONG: {
      size_t offset = p[0] + 256 * p[1];
      p += 2;
      if (!script.Pop()) {
        p = script.byteCode + offset;
      }
      continue;
    }
    case BC::JUMP_IF_NOT_ZERO_SHORT_BEGIN... BC::JUMP_IF_NOT_ZERO_SHORT_END:
      if (script.Pop()) {
        int offset = c + 1 - BC::JUMP_IF_NOT_ZERO_SHORT_BEGIN;
        p += offset;
      }
      continue;
    case BC::JUMP_IF_NOT_ZERO_LONG: {
      size_t offset = p[0] + 256 * p[1];
      p += 2;
      if (script.Pop()) {
        p = script.byteCode + offset;
      }
      continue;
    }
    }
  }
}

void Script::RunConsoleCommand(const char *command) {
  consoleWriter.Reset();

  const uint8_t *result;
  if (Console::RunCommand(command, consoleWriter)) {
    consoleWriter.AddTrailingNull();
    const StenoScriptByteCodeData *bc =
        (const StenoScriptByteCodeData *)byteCode;
    result = bc->FindStringOrReturnOriginal(consoleWriter.buffer);
  } else {
    result = (const uint8_t *)"Invalid console command";
  }

  Push(intptr_t(result - byteCode));
}

void Script::RunGetParameterCommand(const char *parameter) {
  char *command = Str::Join("get_parameter ", parameter, nullptr);
  RunConsoleCommand(command);
  free(command);
}

//---------------------------------------------------------------------------

__attribute__((weak)) void Script::SetInputHint(int hint) {}
__attribute__((weak)) bool Script::IsWaitingForUserPresence() { return false; }
__attribute__((weak)) void Script::ReplyUserPresence(bool present) {}

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
    0x4a, 0x53, 0x53, 0x33, 0x36, 0x00, 0x31, 0x00, 0x34, 0x00, 0x12,
    0x00, 0x1e, 0x00, 0x2a, 0x00, 0x30, 0x00, 0x40, 0x01, 0x77, 0xc3,
    0x04, 0x90, 0x00, 0x92, 0x00, 0x90, 0x04, 0x92, 0x40, 0x01, 0x77,
    0xc3, 0x04, 0x90, 0x01, 0x92, 0x00, 0x90, 0x05, 0x92, 0x90, 0x07,
    0x40, 0x70, 0x48, 0x92, 0x92, 0x00, 0x48, 0x92, 0x92, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

class ScriptTestHelper {
public:
  static void TestGlobalInitializer() {
    Script script(TEST_BYTE_CODE);
    script.globals[0] = 1;
    script.globals[1] = 2;
    script.globals[2] = 3;
    script.globals[3] = 4;
    script.ExecuteInitScript(0);
    assert(script.globals[0] == 0);
  }

  static void TestPress0AndRelease0() {
    Script script(TEST_BYTE_CODE);
    script.ExecuteInitScript(0);

    // Verify S1 is not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());

    script.HandlePress(0, 0);

    // Verify S1 is pressed.
    assert((script.stenoState & StenoKeyState(1)).IsNotEmpty());

    script.HandleRelease(0, 0);

    // Verify S1 is release.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
  }

  static void TestPress1ThenPress0() {
    Script script(TEST_BYTE_CODE);
    script.ExecuteInitScript(0);

    // Verify S1 and A are not presed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
    assert(script.keyState.IsSet(KeyCode::A) == false);

    script.HandlePress(1, 0);
    script.HandlePress(0, 0);

    // Verify S1 is not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());

    // Verify A is pressed.
    assert(script.keyState.IsSet(KeyCode::A));

    script.HandleRelease(0, 0);

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

//---------------------------------------------------------------------------

#include "button_script.h"
#include "writer.h"

#include JAVELIN_BOARD_CONFIG

#include "asset_manager.h"
#include "button_script_manager.h"
#include "clock.h"
#include "console.h"
#include "engine.h"
#include "flash.h"
#include "font/monochrome/font.h"
#include "hal/ble.h"
#include "hal/connection.h"
#include "hal/display.h"
#include "hal/gpio.h"
#include "hal/infrared.h"
#include "hal/mouse.h"
#include "hal/power.h"
#include "hal/rgb.h"
#include "hal/sound.h"
#include "hal/usb_status.h"
#include "key.h"
#include "keyboard_led_status.h"
#include "malloc_allocate.h"
#include "random.h"
#include "split/split_power_override.h"
#include "split/split_usb_status.h"
#include "str.h"
#include "timer_manager.h"
#include "uint16.h"
#include "wpm_tracker.h"

#include <assert.h>
#include <math.h>

//---------------------------------------------------------------------------

struct ImageHeader {
  uint8_t zero;
  ImageFormat format;
  Uint16 version;
  Uint16 width;
  Uint16 height;
};
static_assert(sizeof(ImageHeader) == 8);

//---------------------------------------------------------------------------

class ButtonScript::TimerContext final : public TimerHandler,
                                         public JavelinMallocAllocate {

public:
  TimerContext(ButtonScript *script, size_t offset,
               const ScriptByteCode *byteCode)
      : script(script), offset(offset), byteCode(byteCode) {}

  static const uint32_t BUTTON_SCRIPT_TIMER_TYPE_ID = 0x42555343; // 'BUSC'

  uint32_t GetTypeId() const final { return BUTTON_SCRIPT_TIMER_TYPE_ID; }

  const uint8_t *GetInstructions() const {
    return byteCode->GetScriptData<uint8_t>(offset);
  }

private:
  ButtonScript *script;
  size_t offset;
  const ScriptByteCode *byteCode;

  void Run(intptr_t id) final {
    // Take a local copy of the script
    // executionContext.Run can cause this TimerContext object to be destroyed,
    // and a local copy is needed so that the last line does not corrupt memory.
    ButtonScript *localScript = script;

    // A routine may or may not consume the id, so this code records the
    // top of stack and reinstates it after.
    intptr_t *const startingStackTop = localScript->GetStackTop();

    localScript->Push(id);
    localScript->Run(offset, byteCode);
    localScript->SetStackTop(startingStackTop);
  }

  void OnTimerRemovedFromManager() override final { delete this; }
};

//---------------------------------------------------------------------------

ButtonScript::ButtonScript(const uint8_t *byteCode)
    : Script(byteCode, (void (*const *)(
                           Script &, const ScriptByteCode *))FUNCTION_TABLE) {}

void ButtonScript::Reset() {
  ReleaseAll();
  isScriptRgbEnabled = true;
  inPressAllCount = 0;
  inReleaseAllCount = 0;
  pressCount = 0;
  releaseCount = 0;
  stenoState = 0;
  eventHistory[0] = nullptr;
  eventHistory[1] = nullptr;
  eventHistory[2] = nullptr;
  eventHistory[3] = nullptr;
  Mem::Clear(scriptCallbacks);
  for (void *buffer : buffers) {
    free(buffer);
  }
  buffers.Reset();
  Script::Reset();
}

void ButtonScript::ReleaseAll() {
  for (const size_t keyIndex : keyState) {
    Key::Release(uint32_t(keyIndex));
  }
  keyState.ClearAll();

  stenoState = 0;
  CancelAllStenoKeys();

  for (const size_t mouseButtonIndex : mouseButtonState) {
    Mouse::ReleaseButton(mouseButtonIndex);
  }
  mouseButtonState.ClearAll();
}

void ButtonScript::ExecuteScript(size_t offset, uint32_t scriptTime) {
  this->scriptTime = scriptTime;
  Script::ExecuteScript(offset);
}

void ButtonScript::ExecuteScriptId(ButtonScriptId scriptId,
                                   uint32_t scriptTime) {
  if (scriptId >= ButtonScriptId::COUNT) {
    return;
  }

  this->scriptTime = scriptTime;

  const ScriptCallback &callback = scriptCallbacks[(size_t)scriptId];
  Script::ExecuteScript(callback.byteCode, callback.offset);
}

void ButtonScript::ExecuteScriptIndex(size_t index, uint32_t scriptTime) {
  this->scriptTime = scriptTime;
  Script::ExecuteScriptIndex(index);
}

void ButtonScript::ExecuteScriptIndex(size_t index, uint32_t scriptTime,
                                      const intptr_t *parameters,
                                      size_t parameterCount) {
  this->scriptTime = scriptTime;
  Script::ExecuteScriptIndex(index, parameters, parameterCount);
}

void ButtonScript::ExecuteScriptOffset(size_t offset, uint32_t scriptTime) {
  this->scriptTime = scriptTime;
  Script::ExecuteScript(offset);
}

void ButtonScript::ExecuteScriptCallback(const ScriptByteCode *byteCode,
                                         size_t offset, uint32_t scriptTime) {
  this->scriptTime = scriptTime;
  if (offset != 0) {
    Script::Run(offset, byteCode);
  }
}

void ButtonScript::RemoveScriptTimers() {
  TimerManager::instance.IterateTimers(
      nullptr, [](void *context, int timerId, TimerHandler *handler) {
        if (handler->GetTypeId() == TimerContext::BUTTON_SCRIPT_TIMER_TYPE_ID) {
          TimerManager::instance.StopTimer(timerId, Clock::GetMilliseconds());
        }
      });
}

void ButtonScript::CancelAllScriptsForByteCode(const ScriptByteCode *byteCode,
                                               size_t byteCodeSize) {
  const Interval<const uint8_t *> byteCodeRange(
      byteCode->GetScriptData<uint8_t>(0),
      byteCode->GetScriptData<uint8_t>(byteCodeSize));
  CancelAllCallbacksForByteCode(byteCodeRange);
  CancelAllTimersForByteCode(byteCodeRange);
  CancelAllCombosForByteCode(byteCodeRange);
}

void ButtonScript::CancelAllCallbacksForByteCode(
    const Interval<const uint8_t *> &byteCodeRange) {
  for (size_t i = 0; i < (size_t)ButtonScriptId::COUNT; ++i) {
    const ScriptCallback &callback = scriptCallbacks[i];
    if (callback.offset == 0) {
      continue;
    }

    const uint8_t *p =
        callback.byteCode->GetScriptData<uint8_t>(callback.offset);
    if (byteCodeRange.Contains(p)) {
      scriptCallbacks[i].offset = 0;
      Console::Printf("Cancelled stale scriptId: %zu\n", i);
    }
  }
}

void ButtonScript::CancelAllTimersForByteCode(
    const Interval<const uint8_t *> &byteCodeRange) {
  TimerManager::instance.IterateTimers(
      (void *)&byteCodeRange,
      [](void *context, int timerId, TimerHandler *handler) {
        if (handler->GetTypeId() != TimerContext::BUTTON_SCRIPT_TIMER_TYPE_ID) {
          return;
        }
        TimerContext *timerContext = (TimerContext *)handler;
        const Interval<const uint8_t *> &byteCodeRange =
            *(const Interval<const uint8_t *> *)context;

        if (byteCodeRange.Contains(timerContext->GetInstructions())) {
          Console::Printf("Cancelled stale timerId: %d\n", timerId);
          TimerManager::instance.StopTimer(timerId, Clock::GetMilliseconds());
        }
      });
}

void ButtonScript::CancelAllCombosForByteCode(
    const Interval<const uint8_t *> &byteCodeRange) {
  ButtonScriptManager::GetInstance().CancelAllCombosForByteCode(byteCodeRange);
}

//---------------------------------------------------------------------------

void ButtonScript::PrintInfo() const {
  Console::Printf("Script\n");

  const StenoStroke stroke = stenoState.ToStroke();
  Console::Printf("  Steno State: %D\n", &stenoState, sizeof(stenoState));
  Console::Printf("  Steno Stroke: %t\n", &stroke);
  Console::Printf("  Button State: %D\n", &buttonState, sizeof(buttonState));

  Console::Printf("  Callbacks: [");
  const char *prefix = "";
  for (size_t i = 0; i < (size_t)ButtonScriptId::COUNT; ++i) {
    if (scriptCallbacks[i].offset != 0) {
      Console::Printf("%s%zu: 0x%zx", prefix, i, scriptCallbacks[i].offset);
      prefix = ", ";
    }
  }
  Console::Printf("]\n");
}

//---------------------------------------------------------------------------

class ButtonScript::NullTimerContext final : public TimerHandler,
                                             public JavelinMallocAllocate {
public:
  static NullTimerContext instance;

  void Run(intptr_t id) final {}
};

ButtonScript::NullTimerContext ButtonScript::NullTimerContext::instance;

void ButtonScript::StopTimer(int32_t timerId) {
  TimerManager::instance.StopTimer(timerId, scriptTime);
}

void ButtonScript::StartTimer(int32_t timerId, uint32_t interval,
                              bool isRepeating, size_t offset,
                              const ScriptByteCode *byteCode) {
  TimerHandler *context;
  if (offset) {
    context = new TimerContext(this, offset, byteCode);
  } else {
    context = &NullTimerContext::instance;
  }
  TimerManager::instance.StartTimer(timerId, interval, isRepeating, context,
                                    scriptTime);
}

//---------------------------------------------------------------------------

#if RUN_TESTS
[[gnu::weak]] void ButtonScript::OnStenoKeyPressed() {}
[[gnu::weak]] void ButtonScript::OnStenoKeyReleased() {}
[[gnu::weak]] void ButtonScript::CancelStenoKeys(StenoKeyState state) {}
[[gnu::weak]] void ButtonScript::CancelAllStenoKeys() {}
[[gnu::weak]] bool ButtonScript::ProcessScanCode(int scanCode,
                                                 ScanCodeAction action) {
  return false;
}
[[gnu::weak]] void ButtonScript::SendText(const uint8_t *text) {}
#else
void ButtonScript::SendText(const uint8_t *text) {
#if JAVELIN_USE_EMBEDDED_STENO
  StenoEngine::GetInstance().SendText(text);
#endif
}

bool ButtonScript::ProcessScanCode(int scanCode, ScanCodeAction action) {
#if JAVELIN_USE_EMBEDDED_STENO
  const uint32_t modifiers =
      keyState.GetRange(KeyCode::L_CTRL, KeyCode::L_CTRL + 8)
      << MODIFIER_BIT_SHIFT;
  return StenoEngine::GetInstance().ProcessScanCode(scanCode | modifiers,
                                                    action);
#else
  return false;
#endif
}
#endif

//---------------------------------------------------------------------------

class ButtonScript::Function {
public:
  static void PressScanCode(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
    if (key < 256 && !script.keyState.IsSet(key)) {
      if (!script.ProcessScanCode(key, ScanCodeAction::PRESS)) {
        script.keyState.Set(key);
        Key::Press(key);
      }
    }
  }

  static void ReleaseScanCode(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
    if (key < 256 && script.keyState.IsSet(key)) {
      if (!script.ProcessScanCode(key, ScanCodeAction::RELEASE)) {
        script.keyState.Clear(key);
        Key::Release(key);
      }
    }
  }

  static void TapScanCode(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
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
  }

  static void IsScanCodePressed(ButtonScript &script,
                                const ScriptByteCode *byteCode) {
    const uint32_t key = (uint32_t)script.Pop();
    int isPressed = 0;
    if (key < 256) {
      isPressed = script.keyState.IsSet(key);
    }
    script.Push(isPressed);
  }

  static void PressStenoKey(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    if (stenoKey < (uint32_t)StenoKey::COUNT) {

      script.stenoState |= (1ULL << stenoKey);
      script.OnStenoKeyPressed();
    }
  }

  static void ReleaseStenoKey(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    if (stenoKey < (uint32_t)StenoKey::COUNT) {
      script.stenoState &= ~StenoKeyState(1ULL << stenoKey);
      script.OnStenoKeyReleased();
    }
  }

  static void IsStenoKeyPressed(ButtonScript &script,
                                const ScriptByteCode *byteCode) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    int isPressed = 0;
    if (stenoKey < (uint32_t)StenoKey::COUNT) {
      isPressed = (script.stenoState & (1ULL << stenoKey)).IsNotEmpty();
    }
    script.Push(isPressed);
  }

  static void ReleaseAll(ButtonScript &script, const ScriptByteCode *byteCode) {
    script.ReleaseAll();
  }

  static void IsButtonPressed(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const uint32_t buttonIndex = (uint32_t)script.Pop();
    int isPressed = 0;
    if (buttonIndex < script.buttonState.BIT_COUNT) {
      isPressed = script.buttonState.IsSet(buttonIndex);
    }
    script.Push(isPressed);
  }

  static void PressAll(ButtonScript &script, const ScriptByteCode *byteCode) {
    const uint8_t pressAllCount = script.inPressAllCount;
    script.inPressAllCount = pressAllCount + 1;
    for (const size_t buttonIndex : script.buttonState) {
      script.CallPress(buttonIndex, script.scriptTime);
    }
    script.inPressAllCount = pressAllCount;
  }

  static void SendText(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const uint8_t *text = byteCode->GetScriptData<uint8_t>(offset);
    script.SendText(text);
  }

  static void Console(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const char *command = byteCode->GetScriptData<char>(offset);
    script.RunConsoleCommand(command, byteCode);
  }

  static bool CheckButtonStateInternal(ButtonScript &script,
                                       const uint8_t *text) {
    size_t buttonIndex = 0;
    while (*text) {
      switch (*text) {
      case '0':
        // Must be 0.
        if (script.buttonState.IsSet(buttonIndex)) {
          return false;
        }
        break;

      case ' ':
        // Anything is fine.. skip.
        break;

      default:
        // Must be 1.
        if (!script.buttonState.IsSet(buttonIndex)) {
          return false;
        }
        break;
      }
      ++buttonIndex;
      ++text;
    }
    return true;
  }

  static void CheckButtonState(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const uint8_t *text = byteCode->GetScriptData<uint8_t>(offset);
    script.Push(CheckButtonStateInternal(script, text));
  }

  static void IsInPressAll(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    script.Push(script.inPressAllCount);
  }

  static void SetRgb(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int b = script.Pop() & 0xff;
    const int g = script.Pop() & 0xff;
    const int r = script.Pop() & 0xff;
    const int id = (int)script.Pop();
    if (script.isScriptRgbEnabled || !script.IsInbuiltByteCode(byteCode)) {
      Rgb::SetRgb(id, r, g, b);
    }
  }

  static void GetTime(ButtonScript &script, const ScriptByteCode *byteCode) {
    script.Push(script.scriptTime);
  }

  static void GetLedStatus(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const int index = (int)script.Pop();
    script.Push(Connection::GetActiveKeyboardLedStatus().GetLedStatus(index));
  }

  static void SetGpioPin(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int enable = script.Pop() != 0;
    const int pin = (int)script.Pop();
    Gpio::SetPin(pin, enable);
  }

  static void ClearDisplay(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const int displayId = (int)script.Pop();
    Display::Clear(displayId);
  }

  static void SetAutoDraw(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const int autoDrawId = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetAutoDraw(displayId, autoDrawId);
  }

  static void SetScreenOn(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const bool on = script.Pop() != 0;
    const int displayId = (int)script.Pop();
    Display::SetScreenOn(displayId, on);
  }

  static void SetScreenContrast(ButtonScript &script,
                                const ScriptByteCode *byteCode) {
    const int contrast = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetContrast(displayId, contrast);
  }

  static void DrawPixel(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawPixel(displayId, x, y);
  }

  static void DrawLine(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int y2 = (int)script.Pop();
    const int x2 = (int)script.Pop();
    const int y1 = (int)script.Pop();
    const int x1 = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawLine(displayId, x1, y1, x2, y2);
  }

  static void DrawImage(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();

    if (offset == 0) {
      return;
    }
    const uint8_t *data = byteCode->GetScriptData<uint8_t>(offset);
    ImageFormat format;
    int width;
    int height;
    if (data[0] == 0) {
      static_assert(sizeof(ImageHeader) == 8);
      const ImageHeader *header = (const ImageHeader *)data;
      format = header->format;
      width = header->width.ToUint32();
      height = header->height.ToUint32();
      data = (uint8_t *)(header + 1);
    } else {
      format = ImageFormat::BITMAP;
      width = *data++;
      height = *data++;
    }
    Display::DrawImage(displayId, x, y, width, height, format, data);
  }

  static void DrawText(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const char *text = byteCode->GetScriptData<char>(offset);
    const TextAlignment alignment = (TextAlignment)script.Pop();
    const FontId fontId = (FontId)script.Pop();
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawText(displayId, x, y, fontId, alignment, text);
  }

  static void SetDrawColor(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const int color = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetDrawColor(displayId, color);
  }

  static void DrawRect(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int bottom = (int)script.Pop();
    const int right = (int)script.Pop();
    const int top = (int)script.Pop();
    const int left = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawRect(displayId, left, top, right, bottom);
  }

  static void SetHsv(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int v = (int)script.Pop();
    const int s = (int)script.Pop();
    const int h = (int)script.Pop();
    const int id = (int)script.Pop();
    if (script.isScriptRgbEnabled || !script.IsInbuiltByteCode(byteCode)) {
      Rgb::SetHsv(id, h, s, v);
    }
  }

  static void Rand(ButtonScript &script, const ScriptByteCode *byteCode) {
    script.Push(Random::GenerateUint32());
  }

  static void IsUsbMounted(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    script.Push(UsbStatus::instance.IsConnected() ||
                SplitUsbStatus::instance.IsConnected());
  }

  static void IsUsbSuspended(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    script.Push(UsbStatus::instance.IsSleeping() ||
                SplitUsbStatus::instance.IsSleeping());
  }

  static void GetParameter(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const char *parameter = byteCode->GetScriptData<char>(offset);

    char *command = Str::Join("get_parameter ", parameter);
    script.RunConsoleCommand(command, byteCode);
    free(command);
  }

  static void IsConnected(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const ConnectionId connectionId = (ConnectionId)script.Pop();
    script.Push(Connection::IsConnected(connectionId));
  }

  static void GetActiveConnection(ButtonScript &script,
                                  const ScriptByteCode *byteCode) {
    script.Push((int)Connection::GetActiveConnection());
  }

  static void SetPreferredConnection(ButtonScript &script,
                                     const ScriptByteCode *byteCode) {
    const ConnectionId third = (ConnectionId)script.Pop();
    const ConnectionId second = (ConnectionId)script.Pop();
    const ConnectionId first = (ConnectionId)script.Pop();
    Connection::SetPreferredConnection(first, second, third);
  }

  static void IsPairConnected(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const PairConnectionId pairConnectionId = (PairConnectionId)script.Pop();
    script.Push(Connection::IsPairConnected(pairConnectionId));
  }

  static void StartBlePairing(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    Ble::StartPairing();
  }

  static void GetBleProfile(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    script.Push(Ble::GetProfile());
  }

  static void SetBleProfile(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    Ble::SetProfile((int)script.Pop());
  }

  static void IsHostSleeping(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    script.Push(Connection::IsHostSleeping());
  }

  static void IsMainPowered(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    script.Push(UsbStatus::instance.IsPowered());
  }

  static void IsCharging(ButtonScript &script, const ScriptByteCode *byteCode) {
    script.Push(Power::IsCharging());
  }

  static void GetBatteryPercentage(ButtonScript &script,
                                   const ScriptByteCode *byteCode) {
    script.Push(Power::GetBatteryPercentage());
  }

  static void GetActivePairConnection(ButtonScript &script,
                                      const ScriptByteCode *byteCode) {
    script.Push((int)Connection::GetActivePairConnection());
  }

  static void SetBoardPower(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    Power::SetBoardPower(script.Pop() != 0);
  }

  static void SendEvent(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const char *text = byteCode->GetScriptData<char>(offset);
    for (size_t i = 0; i < EVENT_HISTORY_COUNT - 1; ++i) {
      script.eventHistory[i] = script.eventHistory[i + 1];
    }
    script.eventHistory[EVENT_HISTORY_COUNT - 1] = text;
    if (Console::IsEventEnabled(ConsoleEvent::SCRIPT)) {
      Console::Printf("EV {\"e\":\"c\",\"t\":\"%J\"}\n\n", text);
    }
  }

  static void IsPairPowered(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    script.Push(SplitUsbStatus::instance.IsPowered());
  }

  static void SetInputHint(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    // Deprecated. Just pop the parameter.
    script.Pop();
  }

  static void SetScript(ButtonScript &script, const ScriptByteCode *byteCode) {
    const size_t scriptOffset = script.Pop();
    const ButtonScriptId scriptId = (ButtonScriptId)script.Pop();
    script.SetScript(scriptId, byteCode, scriptOffset);
  }

  static void IsBoardPowered(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    script.Push(Power::IsBoardPowered());
  }

  static void StartTimer(ButtonScript &script, const ScriptByteCode *byteCode) {
    const size_t scriptOffset = script.Pop();
    const bool repeating = script.Pop() != 0;
    const uint32_t interval = (uint32_t)script.Pop();
    const int32_t id = script.Pop() & 0x7fffffff;
    script.StartTimer(id, interval, repeating, scriptOffset, byteCode);
  }

  static void StopTimer(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int32_t id = script.Pop() & 0x7fffffff;
    script.StopTimer(id);
  }

  static void IsTimerActive(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    const int32_t id = script.Pop() & 0x7fffffff;
    script.Push(TimerManager::instance.HasTimer(id));
  }

  static void IsBleProfileConnected(ButtonScript &script,
                                    const ScriptByteCode *byteCode) {
    const uint32_t profileId = (uint32_t)script.Pop();
    script.Push(Ble::IsProfileConnected(profileId));
  }

  static void DisconnectBle(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    Ble::Disconnect();
  }

  static void IsBleProfilePaired(ButtonScript &script,
                                 const ScriptByteCode *byteCode) {
    const uint32_t profileId = (uint32_t)script.Pop();
    script.Push(Ble::IsProfilePaired(profileId));
  }

  static void UnpairBle(ButtonScript &script, const ScriptByteCode *byteCode) {
    Ble::Unpair();
  }

  static void IsBleProfileSleeping(ButtonScript &script,
                                   const ScriptByteCode *byteCode) {
    const uint32_t profileId = (uint32_t)script.Pop();
    script.Push(Ble::IsProfileSleeping(profileId));
  }

  static void IsBleAdvertising(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    script.Push(Ble::IsAdvertising());
  }

  static void IsBleScanning(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    script.Push(Ble::IsScanning());
  }

  static void IsWaitingForUserPresence(ButtonScript &script,
                                       const ScriptByteCode *byteCode) {
    script.Push(ButtonScript::IsWaitingForUserPresence());
  }

  static void ReplyUserPresence(ButtonScript &script,
                                const ScriptByteCode *byteCode) {
    ButtonScript::ReplyUserPresence(script.Pop() != 0);
  }

  static void SetGpioInputPin(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const Gpio::Pull pull = (Gpio::Pull)script.Pop();
    const intptr_t pin = script.Pop();
    Gpio::SetInputPin((int)pin, pull);
  }

  static void ReadGpioPin(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const intptr_t pin = script.Pop();
    script.Push(Gpio::GetPin((int)pin));
  }

  static void DrawLuminanceRange(ButtonScript &script,
                                 const ScriptByteCode *byteCode) {
    const int max = (int)script.Pop();
    const int min = (int)script.Pop();
    const intptr_t offset = script.Pop();
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();

    const uint8_t *data = byteCode->GetScriptData<uint8_t>(offset);
    if (data[0] != 0) {
      return;
    }
    const ImageHeader *header = (const ImageHeader *)data;
    if (header->format != ImageFormat::LUMINANCE8) {
      return;
    }

    const int width = header->width.ToUint32();
    const int height = header->height.ToUint32();
    data = (uint8_t *)(header + 1);

    Display::DrawLuminanceRange(displayId, x, y, width, height, data, min, max);
  }

  static void SetGpioPinDutyCycle(ButtonScript &script,
                                  const ScriptByteCode *byteCode) {
    const int dutyCycle = (int)script.Pop();
    const int pin = (int)script.Pop();
    Gpio::SetPinDutyCycle(pin, dutyCycle);
  }

  static void CancelAllStenoKeys(ButtonScript &script,
                                 const ScriptByteCode *byteCode) {
    script.stenoState = 0;
    script.CancelAllStenoKeys();
  }

  static void CancelStenoKey(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    if (stenoKey < (uint32_t)StenoKey::COUNT) {
      const StenoKeyState state = StenoKeyState(1ULL << stenoKey);
      script.stenoState &= ~state;
      script.CancelStenoKeys(state);
    }
  }

  static void StopSound(ButtonScript &script, const ScriptByteCode *byteCode) {
    Sound::Stop();
  }

  static void PlayFrequency(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    const uint32_t frequency = (uint32_t)script.Pop();
    Sound::PlayFrequency(frequency);
  }

  static void PlaySequence(ButtonScript &script,
                           const ScriptByteCode *byteCode) {

    const intptr_t offset = script.Pop();
    const SoundSequenceData *data =
        byteCode->GetScriptData<SoundSequenceData>(offset);
    Sound::PlaySequence(data);
  }

  static void PlayWaveform(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const uint32_t sampleRate = (uint32_t)script.Pop();
    const uint32_t length = (uint32_t)script.Pop();
    const uint8_t *data = (const uint8_t *)script.Pop();
    Sound::PlayWaveform(data, length, sampleRate);
  }

  static void CallAllReleaseScripts(ButtonScript &script,
                                    const ScriptByteCode *byteCode) {
    const uint8_t releaseAllCount = script.inReleaseAllCount;
    script.inReleaseAllCount = releaseAllCount + 1;
    for (const size_t buttonIndex : script.buttonState) {
      script.CallRelease(buttonIndex, script.scriptTime);
    }
    script.inReleaseAllCount = releaseAllCount;
  }

  static void IsInReleaseAll(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    script.Push(script.inReleaseAllCount);
  }

  static void GetPressCount(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    script.Push(script.pressCount);
  }

  static void GetReleaseCount(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    script.Push(script.releaseCount);
  }

  static void IsStenoJoinNext(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
#if JAVELIN_USE_EMBEDDED_STENO
    script.Push(StenoEngine::GetInstance().IsJoinNext());
#else
    script.Push(0);
#endif
  }

  static void PressButton(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    script.PressButton(script.Pop(), script.scriptTime);
  }

  static void ReleaseButton(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    script.ReleaseButton(script.Pop(), script.scriptTime);
  }

  static void PressMouseButton(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    if (mouseButton < 32 && !script.mouseButtonState.IsSet(mouseButton)) {
      script.mouseButtonState.Set(mouseButton);
      Mouse::PressButton(mouseButton);
    }
  }

  static void ReleaseMouseButton(ButtonScript &script,
                                 const ScriptByteCode *byteCode) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    if (mouseButton < 32 && script.mouseButtonState.IsSet(mouseButton)) {
      script.mouseButtonState.Clear(mouseButton);
      Mouse::ReleaseButton(mouseButton);
    }
  }

  static void TapMouseButton(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    if (mouseButton < 32) {
      if (script.mouseButtonState.IsSet(mouseButton)) {
        script.mouseButtonState.Clear(mouseButton);
      } else {
        Mouse::PressButton(mouseButton);
      }
      Mouse::ReleaseButton(mouseButton);
    }
  }

  static void IsMouseButtonPressed(ButtonScript &script,
                                   const ScriptByteCode *byteCode) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    int isPressed = 0;
    if (mouseButton < 32) {
      isPressed = script.mouseButtonState.IsSet(mouseButton);
    }
    script.Push(isPressed);
  }

  static void MoveMouse(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int32_t dy = (int32_t)script.Pop();
    const int32_t dx = (int32_t)script.Pop();
    Mouse::Move(dx, dy);
  }

  static void VWheelMouse(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const int32_t delta = (int32_t)script.Pop();
    Mouse::VWheel(delta);
  }

  static void SetEnableButtonStates(ButtonScript &script,
                                    const ScriptByteCode *byteCode) {
    ButtonScriptManager::GetInstance().SetAllowButtonStateUpdates(
        script.Pop() != 0);
  }

  static void PrintValue(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t value = script.Pop();
    const intptr_t offset = script.Pop();
    const uint8_t *text = byteCode->GetScriptData<uint8_t>(offset);
    Console::Printf("%s: %zd (0x%zx)\n\n", text, value, value);
  }

  static void GetWpm(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int seconds = (int)script.Pop();
    script.Push(WpmTracker::instance.GetWpm(seconds));
  }

  static void SetPairBoardPower(ButtonScript &script,
                                const ScriptByteCode *byteCode) {
    const PowerOverride powerOverride = (PowerOverride)script.Pop();
    SplitPowerOverride::Set(powerOverride);
  }

  static void HWheelMouse(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    const int32_t delta = (int32_t)script.Pop();
    Mouse::HWheel(delta);
  }

  static void EnableConsole(ButtonScript &script,
                            const ScriptByteCode *byteCode) {
    Console::Enable();
  }
  static void DisableConsole(ButtonScript &script,
                             const ScriptByteCode *byteCode) {
    Console::Disable();
  }
  static void IsConsoleEnabled(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    script.Push(Console::IsEnabled());
  }

  static void EnableFlashWrite(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    // Prevent console scripts from turning off flash write protection.
    if (!script.IsInbuiltByteCode(byteCode)) {
      Console::Printf("ERR protected method\n\n");
      return;
    }

    Flash::EnableWrite();
  }
  static void DisableFlashWrite(ButtonScript &script,
                                const ScriptByteCode *byteCode) {
    Flash::DisableWrite();
  }
  static void IsFlashWriteEnabled(ButtonScript &script,
                                  const ScriptByteCode *byteCode) {
    script.Push(Flash::IsWriteEnabled());
  }

  static void IsInReinit(ButtonScript &script, const ScriptByteCode *byteCode) {
    script.Push(script.isInReinit);
  }

  static void SetDrawColorRgb(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const int b = (int)script.Pop();
    const int g = (int)script.Pop();
    const int r = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetDrawColorRgb(displayId, r, g, b);
  }

  static void SetDrawColorHsv(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    const int v = (int)script.Pop();
    const int s = (int)script.Pop();
    const int h = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetDrawColorHsv(displayId, h, s, v);
  }

  static void DrawEffect(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int parameter = (int)script.Pop();
    const int effectId = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawEffect(displayId, effectId, parameter);
  }

  static void Sin(ButtonScript &script, const ScriptByteCode *byteCode) {
    const float angle = (int)script.Pop() * (M_PI / 32768);
    const float value = sin(angle);
    const int fixedPointValue = int(65536 * value);
    script.Push(fixedPointValue);
  }

  static void Cos(ButtonScript &script, const ScriptByteCode *byteCode) {
    const float angle = (int)script.Pop() * (M_PI / 32768);
    const float value = cos(angle);
    const int fixedPointValue = int(65536 * value);
    script.Push(fixedPointValue);
  }

  static void Tan(ButtonScript &script, const ScriptByteCode *byteCode) {
    const float angle = (int)script.Pop() * (M_PI / 32768);
    const float value = tan(angle);
    const int fixedPointValue = int(65536 * value);
    script.Push(fixedPointValue);
  }

  static void Asin(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int value = (int)script.Pop() / 65536.0f;
    const float angle = asin(value);
    const int fixedPointDegrees = (int)(angle * (32768 / M_PI));
    script.Push(fixedPointDegrees);
  }

  static void Acos(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int value = (int)script.Pop() / 65536.0f;
    const float angle = acos(value);
    const int fixedPointDegrees = (int)(angle * (32768 / M_PI));
    script.Push(fixedPointDegrees);
  }

  static void Atan(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int value = (int)script.Pop() / 65536.0f;
    const float angle = atan(value);
    const int fixedPointDegrees = (int)(angle * (32768 / M_PI));
    script.Push(fixedPointDegrees);
  }

  static void Atan2(ButtonScript &script, const ScriptByteCode *byteCode) {
    const int x = (int)script.Pop();
    const int y = (int)script.Pop();
    if (x == 0 && y == 0) {
      script.Push(0);
      return;
    }
    const float angle = atan2(y, x);
    const int fixedPointDegrees = (int)(angle * (32768 / M_PI));
    script.Push(fixedPointDegrees);
  }

  static void FormatString(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const int value = (int)script.Pop();
    const intptr_t offset = script.Pop();
    const char *text = byteCode->GetScriptData<char>(offset);

    LimitedBufferWriter &writer =
        script.formatStringWriter[script.formatStringWriterIndex];
    script.formatStringWriterIndex = (script.formatStringWriterIndex + 1) & 1;

    writer.Reset();
    writer.Printf(text, value);
    writer.AddTrailingNull();

    const uint8_t *result = byteCode->FindStringOrReturnOriginal(writer.buffer);
    script.Push(byteCode->GetDataOffset(result));
  }

  static void GetAsset(ButtonScript &script, const ScriptByteCode *byteCode) {
    const intptr_t offset = script.Pop();
    const char *assetName = byteCode->GetScriptData<char>(offset);

    const void *data = AssetManager::GetAssetData(assetName);
    const intptr_t dataOffset = data ? byteCode->GetDataOffset(data) : 0;
    script.Push(dataOffset);
  }

  static void AddCombo(ButtonScript &script, const ScriptByteCode *byteCode) {
    const size_t releaseScriptOffset = script.Pop();
    const size_t pressScriptOffset = script.Pop();
    const intptr_t buttonsOffset = script.Pop();
    const int comboTimeOut = (int)script.Pop();
    const bool isOrdered = script.Pop() != 0;

    const uint8_t *buttons = byteCode->GetScriptData<uint8_t>(buttonsOffset);
    ButtonScriptManager::GetInstance().AddCombo(
        isOrdered, comboTimeOut, buttons, byteCode, pressScriptOffset,
        releaseScriptOffset);
  }

  static void ResetCombos(ButtonScript &script,
                          const ScriptByteCode *byteCode) {
    ButtonScriptManager::GetInstance().ResetCombos();
  }

  static void SendInfraredMessage(ButtonScript &script,
                                  const ScriptByteCode *byteCode) {
    const uint32_t d2 = (uint32_t)script.Pop();
    const uint32_t d1 = (uint32_t)script.Pop();
    const uint32_t d0 = (uint32_t)script.Pop();
    const intptr_t protocolNameOffset = script.Pop();

    const char *protocolName =
        protocolNameOffset == 0
            ? ""
            : byteCode->GetScriptData<char>(protocolNameOffset);

    Infrared::SendMessage(protocolName, d0, d1, d2);
  }

  static void SendInfraredData(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    const intptr_t configurationOffset = script.Pop();
    const uint32_t bitCount = (uint32_t)script.Pop();
    const intptr_t dataOffset = script.Pop();

    if (configurationOffset == 0 || bitCount == 0 || dataOffset == 0) {
      return;
    }

    const InfraredDataConfiguration *configuration =
        byteCode->GetScriptData<InfraredDataConfiguration>(configurationOffset);
    const uint8_t *data = byteCode->GetScriptData<uint8_t>(dataOffset);

    Infrared::SendData(data, bitCount, *configuration);
  }

  static void StopInfrared(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    Infrared::Stop();
  }

  static void PrintData(ButtonScript &script, const ScriptByteCode *byteCode) {
    const size_t dataLength = script.Pop();
    const intptr_t dataOffset = script.Pop();
    const intptr_t textOffset = script.Pop();

    const uint8_t *text = byteCode->GetScriptData<uint8_t>(textOffset);
    const void *data = byteCode->GetScriptData<void>(dataOffset);
    Console::Printf("%s: %D\n\n", text, data, dataLength);
  }

  static void MeasureTextWidth(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    const intptr_t textOffset = script.Pop();
    const FontId fontId = (FontId)script.Pop();

    const Font *font = Font::GetFont(fontId);
    if (font == nullptr) {
      script.Push(0);
      return;
    }

    const char *text = byteCode->GetScriptData<char>(textOffset);
    script.Push(font->GetStringWidth(text));
  }

  static void EnableScriptRgb(ButtonScript &script,
                              const ScriptByteCode *byteCode) {
    script.isScriptRgbEnabled = true;
  }

  static void DisableScriptRgb(ButtonScript &script,
                               const ScriptByteCode *byteCode) {
    script.isScriptRgbEnabled = false;
  }

  static void SendInfraredSignal(ButtonScript &script,
                                 const ScriptByteCode *byteCode) {
    const intptr_t configurationOffset = script.Pop();
    const uint32_t dataCount = (uint32_t)script.Pop();
    const intptr_t dataOffset = script.Pop();

    if (configurationOffset == 0 || dataCount == 0 || dataOffset == 0) {
      return;
    }

    const InfraredRawDataConfiguration *configuration =
        byteCode->GetScriptData<InfraredRawDataConfiguration>(
            configurationOffset);
    const InfraredTime *data =
        byteCode->GetScriptData<InfraredTime>(dataOffset);

    Infrared::SendRawData(data, dataCount, *configuration);
  }

  static void CreateBuffer(ButtonScript &script,
                           const ScriptByteCode *byteCode) {
    const size_t bufferSize = script.Pop();

    if (!script.isInInit) {
      script.Push(0);
      return;
    }

    void *buffer = malloc(bufferSize);
    if (buffer == nullptr) {
      script.Push(0);
      return;
    }

    memset(buffer, 0, bufferSize);
    script.buffers.Add(buffer);
    script.Push(byteCode->GetDataOffset(buffer));
  }

  // static void FreeBuffer(ButtonScript &script, const ScriptByteCode
  // *byteCode) {
  //   const size_t bufferOffset = script.Pop();
  //   void *buffer = (void *)byteCode->GetScriptData<void>(bufferOffset);
  //   if (!script.buffers.Remove(buffer)) {
  //     return;
  //   }
  //   free(buffer);
  // }
};

constexpr void (*ButtonScript::FUNCTION_TABLE[])(ButtonScript &,
                                                 const ScriptByteCode *) = {
    &Function::PressScanCode,
    &Function::ReleaseScanCode,
    &Function::TapScanCode,
    &Function::IsScanCodePressed,
    &Function::PressStenoKey,
    &Function::ReleaseStenoKey,
    &Function::IsStenoKeyPressed,
    &Function::ReleaseAll,
    &Function::IsButtonPressed,
    &Function::PressAll,
    &Function::SendText,
    &Function::Console,
    &Function::CheckButtonState,
    &Function::IsInPressAll,
    &Function::SetRgb,
    &Function::GetTime,
    &Function::GetLedStatus,
    &Function::SetGpioPin,
    &Function::ClearDisplay,
    &Function::SetAutoDraw,
    &Function::SetScreenOn,
    &Function::SetScreenContrast,
    &Function::DrawPixel,
    &Function::DrawLine,
    &Function::DrawImage,
    &Function::DrawText,
    &Function::SetDrawColor,
    &Function::DrawRect,
    &Function::SetHsv,
    &Function::Rand,
    &Function::IsUsbMounted,
    &Function::IsUsbSuspended,
    &Function::GetParameter,
    &Function::IsConnected,
    &Function::GetActiveConnection,
    &Function::SetPreferredConnection,
    &Function::IsPairConnected,
    &Function::StartBlePairing,
    &Function::GetBleProfile,
    &Function::SetBleProfile,
    &Function::IsHostSleeping,
    &Function::IsMainPowered,
    &Function::IsCharging,
    &Function::GetBatteryPercentage,
    &Function::GetActivePairConnection,
    &Function::SetBoardPower,
    &Function::SendEvent,
    &Function::IsPairPowered,
    &Function::SetInputHint,
    &Function::SetScript,
    &Function::IsBoardPowered,
    &Function::StartTimer,
    &Function::StopTimer,
    &Function::IsTimerActive,
    &Function::IsBleProfileConnected,
    &Function::DisconnectBle,
    &Function::UnpairBle,
    &Function::IsBleProfilePaired,
    &Function::IsBleProfileSleeping,
    &Function::IsBleAdvertising,
    &Function::IsBleScanning,
    &Function::IsWaitingForUserPresence,
    &Function::ReplyUserPresence,
    &Function::SetGpioInputPin,
    &Function::ReadGpioPin,
    &Function::DrawLuminanceRange,
    &Function::SetGpioPinDutyCycle,
    &Function::CancelAllStenoKeys,
    &Function::CancelStenoKey,
    &Function::StopSound,
    &Function::PlayFrequency,
    &Function::PlaySequence,
    &Function::PlayWaveform,
    &Function::CallAllReleaseScripts,
    &Function::IsInReleaseAll,
    &Function::GetPressCount,
    &Function::GetReleaseCount,
    &Function::IsStenoJoinNext,
    &Function::PressButton,
    &Function::ReleaseButton,
    &Function::PressMouseButton,
    &Function::ReleaseMouseButton,
    &Function::TapMouseButton,
    &Function::IsMouseButtonPressed,
    &Function::MoveMouse,
    &Function::VWheelMouse,
    &Function::SetEnableButtonStates,
    &Function::PrintValue,
    &Function::GetWpm,
    &Function::SetPairBoardPower,
    &Function::HWheelMouse,
    &Function::EnableConsole,
    &Function::DisableConsole,
    &Function::IsConsoleEnabled,
    &Function::EnableFlashWrite,
    &Function::DisableFlashWrite,
    &Function::IsFlashWriteEnabled,
    &Function::IsInReinit,
    &Function::SetDrawColorRgb,
    &Function::SetDrawColorHsv,
    &Function::DrawEffect,
    &Function::Sin,
    &Function::Cos,
    &Function::Tan,
    &Function::Asin,
    &Function::Acos,
    &Function::Atan,
    &Function::Atan2,
    &Function::FormatString,
    &Function::GetAsset,
    &Function::AddCombo,
    &Function::ResetCombos,
    &Function::SendInfraredMessage,
    &Function::SendInfraredData,
    &Function::StopInfrared,
    &Function::PrintData,
    &Function::MeasureTextWidth,
    &Function::EnableScriptRgb,
    &Function::DisableScriptRgb,
    &Function::SendInfraredSignal,
    &Function::CreateBuffer,
    // &Function::FreeBuffer,
};

void ButtonScript::PrintEventHistory() {
  Console::Printf("[");
  bool isFirst = true;
  for (const char *event : eventHistory) {
    const char *format = event ? ",\"%J\"" : ",null";
    Console::Printf(format + isFirst, event);
    isFirst = false;
  }
  Console::Printf("]\n\n");
}

void ButtonScript::RunConsoleCommand(const char *command,
                                     const ScriptByteCode *byteCode) {
  consoleWriter.Reset();

  const uint8_t *result;
  if (Console::RunCommand(command, consoleWriter)) {
    consoleWriter.AddTrailingNull();
    result = byteCode->FindStringOrReturnOriginal(consoleWriter.buffer);
  } else {
    result = (const uint8_t *)"Invalid console command";
  }

  Push(byteCode->GetDataOffset(result));
}

//---------------------------------------------------------------------------

[[gnu::weak]] bool ButtonScript::IsWaitingForUserPresence() { return false; }
[[gnu::weak]] void ButtonScript::ReplyUserPresence(bool present) {}

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

constexpr uint8_t TEST_BYTE_CODE[] = {
    0x4a, 0x53, 0x53, 0x33, 0x36, 0x00, 0x31, 0x00, 0x34, 0x00, 0x12,
    0x00, 0x1e, 0x00, 0x2a, 0x00, 0x30, 0x00, 0x40, 0x01, 0x77, 0xc3,
    0x04, 0x90, 0x00, 0x92, 0x00, 0x90, 0x04, 0x92, 0x40, 0x01, 0x77,
    0xc3, 0x04, 0x90, 0x01, 0x92, 0x00, 0x90, 0x05, 0x92, 0x90, 0x07,
    0x40, 0x70, 0x48, 0x92, 0x92, 0x00, 0x48, 0x92, 0x92, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

class ButtonScript::TestHelper {
public:
  static void TestGlobalInitializer() {
    ButtonScript script(TEST_BYTE_CODE);
    script.SetGlobal(0, 1);
    script.SetGlobal(1, 2);
    script.SetGlobal(2, 3);
    script.SetGlobal(3, 4);
    script.ExecuteInitScript(0);
    assert(script.GetGlobal(0) == 0);
  }

  static void TestPress0AndRelease0() {
    ButtonScript script(TEST_BYTE_CODE);
    script.keyState.ClearAll();
    script.ExecuteInitScript(0);

    // Verify S1 is not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());

    script.PressButton(0, 0);

    // Verify S1 is pressed.
    assert((script.stenoState & StenoKeyState(1)).IsNotEmpty());

    script.ReleaseButton(0, 0);

    // Verify S1 is release.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
  }

  static void TestPress1ThenPress0() {
    ButtonScript script(TEST_BYTE_CODE);
    script.keyState.ClearAll();
    script.ExecuteInitScript(0);

    // Verify S1 and A are not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
    assert(script.keyState.IsSet(KeyCode::A) == false);

    script.PressButton(1, 0);
    script.PressButton(0, 0);

    // Verify S1 is not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());

    // Verify A is pressed.
    assert(script.keyState.IsSet(KeyCode::A));

    script.ReleaseButton(0, 0);

    // Verify S1 and A are not pressed.
    assert((script.stenoState & StenoKeyState(1)).IsEmpty());
    assert(script.keyState.IsSet(KeyCode::A) == false);
  }
};

TEST_BEGIN("ButtonScript: Initializes globals correctly") {
  ButtonScript::TestHelper::TestGlobalInitializer();
}
TEST_END

TEST_BEGIN("ButtonScript: Press0 and Release0 causes S1 to toggle") {
  ButtonScript::TestHelper::TestPress0AndRelease0();
}
TEST_END

TEST_BEGIN("ButtonScript: Press1 then Press0 causes A to be pressed") {
  ButtonScript::TestHelper::TestPress1ThenPress0();
}
TEST_END

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "button_script.h"

#include JAVELIN_BOARD_CONFIG

#include "button_script_manager.h"
#include "console.h"
#include "engine.h"
#include "hal/ble.h"
#include "hal/connection.h"
#include "hal/display.h"
#include "hal/gpio.h"
#include "hal/mouse.h"
#include "hal/power.h"
#include "hal/rgb.h"
#include "hal/sound.h"
#include "hal/usb_status.h"
#include "key.h"
#include "keyboard_led_status.h"
#include "malloc_allocate.h"
#include "random.h"
#include "split/split_usb_status.h"
#include "str.h"
#include "timer_manager.h"
#include "wpm_tracker.h"

#include <assert.h>

//---------------------------------------------------------------------------

ButtonScript::ButtonScript(const uint8_t *byteCode)
    : Script(byteCode, (void (*const *)(Script &))FUNCTION_TABLE) {
  keyState.ClearAll();
}

void ButtonScript::Reset() {
  ReleaseAll();
  inPressAllCount = 0;
  inReleaseAllCount = 0;
  pressCount = 0;
  releaseCount = 0;
  stenoState = 0;
  eventHistory[0] = nullptr;
  eventHistory[1] = nullptr;
  eventHistory[2] = nullptr;
  eventHistory[3] = nullptr;
  Mem::Clear(scriptOffsets);
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

  const size_t offset = scriptOffsets[(size_t)scriptId];
  ExecuteScript(offset, scriptTime);
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

//---------------------------------------------------------------------------

void ButtonScript::PrintInfo() const {
  Console::Printf("Script\n");

  Console::Printf("  Callbacks: [");
  const char *prefix = "";
  for (size_t i = 0; i < (size_t)ButtonScriptId::COUNT; ++i) {
    if (scriptOffsets[i] != 0) {
      Console::Printf("%s%zu: 0x%zx", prefix, i, scriptOffsets[i]);
      prefix = ", ";
    }
  }
  Console::Printf("]\n");
}

//---------------------------------------------------------------------------

class ButtonScript::TimerContext final : public TimerHandler,
                                         public JavelinMallocAllocate {

public:
  TimerContext(ButtonScript *script, size_t offset)
      : script(script), offset(offset) {}

private:
  ButtonScript *script;
  size_t offset;

  void Run(intptr_t id) final {
    // Take a local copy of the script
    // executionContext.Run can cause this TimerContext object to be destroyed,
    // and a local copy is needed so that the last line does not corrupt memory.
    ButtonScript *localScript = script;

    // A routine may or may not consume the id, so this code records the
    // top of stack and reinstates it after.
    intptr_t *const startingStackTop = localScript->GetStackTop();

    localScript->Push(id);
    localScript->Run(offset);
    localScript->SetStackTop(startingStackTop);
  }

  void OnTimerRemovedFromManager() override final { delete this; }
};

void ButtonScript::StopTimer(int32_t timerId) {
  TimerManager::instance.StopTimer(timerId, scriptTime);
}

void ButtonScript::StartTimer(int32_t timerId, uint32_t interval,
                              bool isRepeating, size_t offset) {
  TimerContext *context = new TimerContext(this, offset);
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
  static void PressScanCode(ButtonScript &script) {
    const uint32_t key = (uint32_t)script.Pop();
    if (key < 256 && !script.keyState.IsSet(key)) {
      if (!script.ProcessScanCode(key, ScanCodeAction::PRESS)) {
        script.keyState.Set(key);
        Key::Press(key);
      }
    }
  }

  static void ReleaseScanCode(ButtonScript &script) {
    const uint32_t key = (uint32_t)script.Pop();
    if (key < 256 && script.keyState.IsSet(key)) {
      if (!script.ProcessScanCode(key, ScanCodeAction::RELEASE)) {
        script.keyState.Clear(key);
        Key::Release(key);
      }
    }
  }

  static void TapScanCode(ButtonScript &script) {
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

  static void IsScanCodePressed(ButtonScript &script) {
    const uint32_t key = (uint32_t)script.Pop();
    int isPressed = 0;
    if (key < 256) {
      isPressed = script.keyState.IsSet(key);
    }
    script.Push(isPressed);
  }

  static void PressStenoKey(ButtonScript &script) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    if (stenoKey < (uint32_t)StenoKey::COUNT) {

      script.stenoState |= (1ULL << stenoKey);
      script.OnStenoKeyPressed();
    }
  }

  static void ReleaseStenoKey(ButtonScript &script) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    if (stenoKey < (uint32_t)StenoKey::COUNT) {
      script.stenoState &= ~StenoKeyState(1ULL << stenoKey);
      script.OnStenoKeyReleased();
    }
  }

  static void IsStenoKeyPressed(ButtonScript &script) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    int isPressed = 0;
    if (stenoKey < (uint32_t)StenoKey::COUNT) {
      isPressed = (script.stenoState & (1ULL << stenoKey)).IsNotEmpty();
    }
    script.Push(isPressed);
  }

  static void ReleaseAll(ButtonScript &script) { script.ReleaseAll(); }

  static void IsButtonPressed(ButtonScript &script) {
    const uint32_t buttonIndex = (uint32_t)script.Pop();
    int isPressed = 0;
    if (buttonIndex < script.buttonState.BIT_COUNT) {
      isPressed = script.buttonState.IsSet(buttonIndex);
    }
    script.Push(isPressed);
  }

  static void PressAll(ButtonScript &script) {
    const uint8_t pressAllCount = script.inPressAllCount;
    script.inPressAllCount = pressAllCount + 1;
    for (const size_t buttonIndex : script.buttonState) {
      script.CallPress(buttonIndex, script.scriptTime);
    }
    script.inPressAllCount = pressAllCount;
  }

  static void SendText(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const uint8_t *text = script.GetScriptData<uint8_t>(offset);
    script.SendText(text);
  }

  static void Console(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const char *command = script.GetScriptData<char>(offset);
    script.RunConsoleCommand(command);
  }

  static bool CheckButtonState(ButtonScript &script, const uint8_t *text) {
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

  static void CheckButtonState(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const uint8_t *text = script.GetScriptData<uint8_t>(offset);
    script.Push(CheckButtonState(script, text));
  }

  static void IsInPressAll(ButtonScript &script) {
    script.Push(script.inPressAllCount);
  }

  static void SetRgb(ButtonScript &script) {
    const int b = script.Pop() & 0xff;
    const int g = script.Pop() & 0xff;
    const int r = script.Pop() & 0xff;
    const int id = (int)script.Pop();
    Rgb::SetRgb(id, r, g, b);
  }

  static void GetTime(ButtonScript &script) { script.Push(script.scriptTime); }

  static void GetLedStatus(ButtonScript &script) {
    const int index = (int)script.Pop();
    script.Push(Connection::GetActiveKeyboardLedStatus().GetLedStatus(index));
  }

  static void SetGpioPin(ButtonScript &script) {
    const int enable = script.Pop() != 0;
    const int pin = (int)script.Pop();
    Gpio::SetPin(pin, enable);
  }

  static void ClearDisplay(ButtonScript &script) {
    const int displayId = (int)script.Pop();
    Display::Clear(displayId);
  }

  static void SetAutoDraw(ButtonScript &script) {
    const int autoDrawId = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetAutoDraw(displayId, autoDrawId);
  }

  static void SetScreenOn(ButtonScript &script) {
    const bool on = script.Pop() != 0;
    const int displayId = (int)script.Pop();
    Display::SetScreenOn(displayId, on);
  }

  static void SetScreenContrast(ButtonScript &script) {
    const int contrast = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetContrast(displayId, contrast);
  }

  static void DrawPixel(ButtonScript &script) {
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawPixel(displayId, x, y);
  }

  static void DrawLine(ButtonScript &script) {
    const int y2 = (int)script.Pop();
    const int x2 = (int)script.Pop();
    const int y1 = (int)script.Pop();
    const int x1 = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawLine(displayId, x1, y1, x2, y2);
  }

  static void DrawImage(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const uint8_t *data = script.GetScriptData<uint8_t>(offset);
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();
    const int width = *data++;
    const int height = *data++;
    Display::DrawImage(displayId, x, y, width, height, data);
  }

  static void DrawText(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const char *text = script.GetScriptData<char>(offset);
    const TextAlignment alignment = (TextAlignment)script.Pop();
    const FontId fontId = (FontId)script.Pop();
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawText(displayId, x, y, fontId, alignment, text);
  }

  static void SetDrawColor(ButtonScript &script) {
    const int color = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::SetDrawColor(displayId, color);
  }

  static void DrawRect(ButtonScript &script) {
    const int bottom = (int)script.Pop();
    const int right = (int)script.Pop();
    const int top = (int)script.Pop();
    const int left = (int)script.Pop();
    const int displayId = (int)script.Pop();
    Display::DrawRect(displayId, left, top, right, bottom);
  }

  static void SetHsv(ButtonScript &script) {
    const int v = (int)script.Pop();
    const int s = (int)script.Pop();
    const int h = (int)script.Pop();
    const int id = (int)script.Pop();
    Rgb::SetHsv(id, h, s, v);
  }

  static void Rand(ButtonScript &script) {
    script.Push(Random::GenerateUint32());
  }

  static void IsUsbMounted(ButtonScript &script) {
    script.Push(UsbStatus::instance.IsConnected() ||
                SplitUsbStatus::instance.IsConnected());
  }

  static void IsUsbSuspended(ButtonScript &script) {
    script.Push(UsbStatus::instance.IsSleeping() ||
                SplitUsbStatus::instance.IsSleeping());
  }

  static void GetParameter(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const char *parameter = script.GetScriptData<char>(offset);

    char *command = Str::Join("get_parameter ", parameter);
    script.RunConsoleCommand(command);
    free(command);
  }

  static void IsConnected(ButtonScript &script) {
    const ConnectionId connectionId = (ConnectionId)script.Pop();
    script.Push(Connection::IsConnected(connectionId));
  }

  static void GetActiveConnection(ButtonScript &script) {
    script.Push((int)Connection::GetActiveConnection());
  }

  static void SetPreferredConnection(ButtonScript &script) {
    const ConnectionId third = (ConnectionId)script.Pop();
    const ConnectionId second = (ConnectionId)script.Pop();
    const ConnectionId first = (ConnectionId)script.Pop();
    Connection::SetPreferredConnection(first, second, third);
  }

  static void IsPairConnected(ButtonScript &script) {
    const PairConnectionId pairConnectionId = (PairConnectionId)script.Pop();
    script.Push(Connection::IsPairConnected(pairConnectionId));
  }

  static void StartBlePairing(ButtonScript &script) { Ble::StartPairing(); }

  static void GetBleProfile(ButtonScript &script) {
    script.Push(Ble::GetProfile());
  }

  static void SetBleProfile(ButtonScript &script) {
    Ble::SetProfile((int)script.Pop());
  }

  static void IsHostSleeping(ButtonScript &script) {
    script.Push(Connection::IsHostSleeping());
  }

  static void IsMainPowered(ButtonScript &script) {
    script.Push(UsbStatus::instance.IsPowered());
  }

  static void IsCharging(ButtonScript &script) {
    script.Push(Power::IsCharging());
  }

  static void GetBatteryPercentage(ButtonScript &script) {
    script.Push(Power::GetBatteryPercentage());
  }

  static void GetActivePairConnection(ButtonScript &script) {
    script.Push((int)Connection::GetActivePairConnection());
  }

  static void SetBoardPower(ButtonScript &script) {
    Power::SetBoardPower(script.Pop() != 0);
  }

  static void SendEvent(ButtonScript &script) {
    const intptr_t offset = script.Pop();
    const char *text = script.GetScriptData<char>(offset);
    for (size_t i = 0; i < EVENT_HISTORY_COUNT - 1; ++i) {
      script.eventHistory[i] = script.eventHistory[i + 1];
    }
    script.eventHistory[EVENT_HISTORY_COUNT - 1] = text;
    if (script.scriptEventsEnabled) {
      Console::WriteScriptEvent(text);
    }
  }

  static void IsPairPowered(ButtonScript &script) {
    script.Push(SplitUsbStatus::instance.IsPowered());
  }

  static void SetInputHint(ButtonScript &script) {
    // Deprecated. Just pop the parameter.
    script.Pop();
  }

  static void SetScript(ButtonScript &script) {
    const size_t scriptOffset = script.Pop();
    const ButtonScriptId scriptId = (ButtonScriptId)script.Pop();
    script.SetScript(scriptId, scriptOffset);
  }

  static void IsBoardPowered(ButtonScript &script) {
    script.Push(Power::IsBoardPowered());
  }

  static void StartTimer(ButtonScript &script) {
    const size_t scriptOffset = script.Pop();
    const bool repeating = script.Pop() != 0;
    const uint32_t interval = (uint32_t)script.Pop();
    const int32_t id = script.Pop() & 0x7fffffff;
    script.StartTimer(id, interval, repeating, scriptOffset);
  }

  static void StopTimer(ButtonScript &script) {
    const int32_t id = script.Pop() & 0x7fffffff;
    script.StopTimer(id);
  }

  static void IsTimerActive(ButtonScript &script) {
    const int32_t id = script.Pop() & 0x7fffffff;
    script.Push(TimerManager::instance.HasTimer(id));
  }

  static void IsBleProfileConnected(ButtonScript &script) {
    const uint32_t profileId = (uint32_t)script.Pop();
    script.Push(Ble::IsProfileConnected(profileId));
  }

  static void DisconnectBle(ButtonScript &script) { Ble::Disconnect(); }

  static void IsBleProfilePaired(ButtonScript &script) {
    const uint32_t profileId = (uint32_t)script.Pop();
    script.Push(Ble::IsProfilePaired(profileId));
  }

  static void UnpairBle(ButtonScript &script) { Ble::Unpair(); }

  static void IsBleProfileSleeping(ButtonScript &script) {
    const uint32_t profileId = (uint32_t)script.Pop();
    script.Push(Ble::IsProfileSleeping(profileId));
  }

  static void IsBleAdvertising(ButtonScript &script) {
    script.Push(Ble::IsAdvertising());
  }

  static void IsBleScanning(ButtonScript &script) {
    script.Push(Ble::IsScanning());
  }

  static void IsWaitingForUserPresence(ButtonScript &script) {
    script.Push(ButtonScript::IsWaitingForUserPresence());
  }

  static void ReplyUserPresence(ButtonScript &script) {
    ButtonScript::ReplyUserPresence(script.Pop() != 0);
  }

  static void SetGpioInputPin(ButtonScript &script) {
    const Gpio::Pull pull = (Gpio::Pull)script.Pop();
    const intptr_t pin = script.Pop();
    Gpio::SetInputPin((int)pin, pull);
  }

  static void ReadGpioPin(ButtonScript &script) {
    const intptr_t pin = script.Pop();
    script.Push(Gpio::GetPin((int)pin));
  }

  static void DrawGrayscaleRange(ButtonScript &script) {
    const int max = (int)script.Pop();
    const int min = (int)script.Pop();
    const intptr_t offset = script.Pop();
    const uint8_t *data = script.GetScriptData<uint8_t>(offset);
    const int y = (int)script.Pop();
    const int x = (int)script.Pop();
    const int displayId = (int)script.Pop();
    const int width = *data++;
    const int height = *data++;
    Display::DrawGrayscaleRange(displayId, x, y, width, height, data, min, max);
  }

  static void SetGpioPinDutyCycle(ButtonScript &script) {
    const int dutyCycle = (int)script.Pop();
    const int pin = (int)script.Pop();
    Gpio::SetPinDutyCycle(pin, dutyCycle);
  }

  static void CancelAllStenoKeys(ButtonScript &script) {
    script.stenoState = 0;
    script.CancelAllStenoKeys();
  }

  static void CancelStenoKey(ButtonScript &script) {
    const uint32_t stenoKey = (uint32_t)script.Pop();
    if (stenoKey < (uint32_t)StenoKey::COUNT) {
      const StenoKeyState state = StenoKeyState(1ULL << stenoKey);
      script.stenoState &= ~state;
      script.CancelStenoKeys(state);
    }
  }

  static void StopSound(ButtonScript &script) { Sound::Stop(); }

  static void PlayFrequency(ButtonScript &script) {
    const uint32_t frequency = (uint32_t)script.Pop();
    Sound::PlayFrequency(frequency);
  }

  static void PlaySequence(ButtonScript &script) {

    const intptr_t offset = script.Pop();
    const SoundSequenceData *data =
        script.GetScriptData<SoundSequenceData>(offset);
    Sound::PlaySequence(data);
  }

  static void PlayWaveform(ButtonScript &script) {
    const uint32_t sampleRate = (uint32_t)script.Pop();
    const uint32_t length = (uint32_t)script.Pop();
    const uint8_t *data = (const uint8_t *)script.Pop();
    Sound::PlayWaveform(data, length, sampleRate);
  }

  static void CallAllReleaseScripts(ButtonScript &script) {
    const uint8_t releaseAllCount = script.inReleaseAllCount;
    script.inReleaseAllCount = releaseAllCount + 1;
    for (const size_t buttonIndex : script.buttonState) {
      script.CallRelease(buttonIndex, script.scriptTime);
    }
    script.inReleaseAllCount = releaseAllCount;
  }

  static void IsInReleaseAll(ButtonScript &script) {
    script.Push(script.inReleaseAllCount);
  }

  static void GetPressCount(ButtonScript &script) {
    script.Push(script.pressCount);
  }

  static void GetReleaseCount(ButtonScript &script) {
    script.Push(script.releaseCount);
  }

  static void IsStenoJoinNext(ButtonScript &script) {
#if JAVELIN_USE_EMBEDDED_STENO
    script.Push(StenoEngine::GetInstance().IsJoinNext());
#else
    script.Push(0);
#endif
  }

  static void CallPress(ButtonScript &script) {
    script.CallPress(script.Pop(), script.scriptTime);
  }

  static void CallRelease(ButtonScript &script) {
    script.CallPress(script.Pop(), script.scriptTime);
  }

  static void PressMouseButton(ButtonScript &script) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    if (mouseButton < 32 && !script.mouseButtonState.IsSet(mouseButton)) {
      script.mouseButtonState.Set(mouseButton);
      Mouse::PressButton(mouseButton);
    }
  }

  static void ReleaseMouseButton(ButtonScript &script) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    if (mouseButton < 32 && script.mouseButtonState.IsSet(mouseButton)) {
      script.mouseButtonState.Clear(mouseButton);
      Mouse::ReleaseButton(mouseButton);
    }
  }

  static void TapMouseButton(ButtonScript &script) {
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

  static void IsMouseButtonPressed(ButtonScript &script) {
    const uint32_t mouseButton = (uint32_t)script.Pop();
    int isPressed = 0;
    if (mouseButton < 32) {
      isPressed = script.mouseButtonState.IsSet(mouseButton);
    }
    script.Push(isPressed);
  }

  static void MoveMouse(ButtonScript &script) {
    const int32_t dy = (int32_t)script.Pop();
    const int32_t dx = (int32_t)script.Pop();
    Mouse::Move(dx, dy);
  }

  static void WheelMouse(ButtonScript &script) {
    const int32_t delta = (int32_t)script.Pop();
    Mouse::Wheel(delta);
  }

  static void SetEnableButtonStates(ButtonScript &script) {
    ButtonScriptManager::GetInstance().SetAllowButtonStateUpdates(
        script.Pop() != 0);
  }

  static void PrintValue(ButtonScript &script) {
    const intptr_t value = script.Pop();
    const intptr_t offset = script.Pop();
    const uint8_t *text = script.GetScriptData<uint8_t>(offset);
    Console::Printf("%s: %zd (0x%zx)\n\n", text, value, value);
  }

  static void GetWpm(ButtonScript &script) {
    const int seconds = (int)script.Pop();
    script.Push(WpmTracker::instance.GetWpm(seconds));
  }
};

constexpr void (*ButtonScript::FUNCTION_TABLE[])(ButtonScript &) = {
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
    &Function::DrawGrayscaleRange,
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
    &Function::CallPress,
    &Function::CallRelease,
    &Function::PressMouseButton,
    &Function::ReleaseMouseButton,
    &Function::TapMouseButton,
    &Function::IsMouseButtonPressed,
    &Function::MoveMouse,
    &Function::WheelMouse,
    &Function::SetEnableButtonStates,
    &Function::PrintValue,
    &Function::GetWpm,
};

void ButtonScript::PrintEventHistory() {
  Console::Printf("[");
  bool first = true;
  for (const char *event : eventHistory) {
    if (first) {
      first = false;
    } else {
      Console::Printf(",");
    }
    if (event) {
      Console::Printf("\"%J\"", event);
    } else {
      Console::Printf("null");
    }
  }
  Console::Printf("]\n\n");
}

void ButtonScript::RunConsoleCommand(const char *command) {
  consoleWriter.Reset();

  const uint8_t *result;
  if (Console::RunCommand(command, consoleWriter)) {
    consoleWriter.AddTrailingNull();
    result = FindStringOrReturnOriginal(consoleWriter.buffer);
  } else {
    result = (const uint8_t *)"Invalid console command";
  }

  PushDataOffset(result);
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

const uint8_t TEST_BYTE_CODE[] = {
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
    ButtonScript script(TEST_BYTE_CODE);
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
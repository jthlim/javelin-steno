//---------------------------------------------------------------------------

#pragma once
#include "bit_field.h"
#include "button_state.h"
#include "scan_code_action.h"
#include "script.h"
#include "steno_key_state.h"
#include "writer.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum class ButtonScriptId : int {
  DISPLAY_OVERLAY,
  BATTERY_UPDATE,
  CONNECTION_UPDATE,
  PAIR_CONNECTION_UPDATE,
  KEYBOARD_LED_STATUS_UPDATE,
  BLE_ADVERTISING_UPDATE,
  BLE_SCANNING_UPDATE,
  U2F_STATUS_UPDATE,
  U2F_WINK,
  STENO_MODE_UPDATE,
  BLE_PAIR_COMPLETE,

  USER1,
  USER2,
  USER3,
  USER4,
  USER5,
  USER6,
  USER7,
  USER8,

  CONSOLE_ACCESS_REQUESTED,
  FLASH_WRITE_ACCESS_REQUESTED,

  COUNT,
};

//---------------------------------------------------------------------------

class ButtonScript : public Script {
public:
  ButtonScript(const uint8_t *byteCode);

  void ExecuteInitScript(uint32_t scriptTime) {
    ExecuteScriptIndex(0, scriptTime);
  }
  void ExecuteTickScript(uint32_t scriptTime) {
    ExecuteScriptIndex(1, scriptTime);
  }
  void ExecuteScriptIndex(size_t index, uint32_t scriptTime);
  void ExecuteScriptIndex(size_t index, uint32_t scriptTime,
                          const intptr_t *parameters, size_t parameterCount);
  void ExecuteScriptOffset(size_t offset, uint32_t scriptTime);
  void ExecuteScriptCallback(const ScriptByteCode *byteCode, size_t offset,
                             uint32_t scriptTime);

  void ExecuteScriptId(ButtonScriptId scriptId, uint32_t scriptTime);

  void PressButton(size_t keyIndex, uint32_t scriptTime) {
    ++pressCount;
    buttonState.Set(keyIndex);
    CallPress(keyIndex, scriptTime);
  }

  void ReleaseButton(size_t keyIndex, uint32_t scriptTime) {
    ++releaseCount;
    buttonState.Clear(keyIndex);
    CallRelease(keyIndex, scriptTime);
  }

  void PrintInfo() const;

  bool IsTickScriptEmpty() const { return IsScriptIndexEmpty(1); }

  static bool IsWaitingForUserPresence();
  static void ReplyUserPresence(bool present);

  void PrintEventHistory();

  void Reset();
  void SetReinit(bool value) { isInReinit = value; }

  static void RemoveScriptTimers();
  void CancelAllScriptsForByteCode(const ScriptByteCode *byteCode,
                                   size_t byteCodeSize);

  class TestHelper;

private:
  class Function;
  class TimerContext;
  class NullTimerContext;

  static constexpr size_t EVENT_HISTORY_COUNT = 4;
  static constexpr size_t MAX_STACK_SIZE = 256;
  static void (*const FUNCTION_TABLE[])(ButtonScript &, const ScriptByteCode *);

  struct ScriptCallback {
    const ScriptByteCode *byteCode;
    size_t offset;

    void Set(const ScriptByteCode *byteCode, size_t offset) {
      this->byteCode = byteCode;
      this->offset = offset;
    }
  };

  bool isInReinit;
  uint8_t inPressAllCount;
  uint8_t inReleaseAllCount;
  uint32_t pressCount;
  uint32_t releaseCount;
  uint32_t scriptTime;
  StenoKeyState stenoState;
  const char *eventHistory[EVENT_HISTORY_COUNT];
  ScriptCallback scriptCallbacks[(size_t)ButtonScriptId::COUNT];
  ButtonState buttonState;
  BitField<256> keyState;
  BitField<32> mouseButtonState;
  LimitedBufferWriter consoleWriter;

  void ExecuteScript(size_t offset, uint32_t scriptTime);

  bool IsScriptEmpty(size_t offset) const;

  void StartTimer(int32_t timerId, uint32_t interval, bool isRepeating,
                  size_t offset, const ScriptByteCode *byteCode);
  void StopTimer(int32_t timerId);

  friend class ScriptTestHelper;

  void OnStenoKeyPressed();
  void OnStenoKeyReleased();
  void CancelStenoKeys(StenoKeyState state);
  void CancelAllStenoKeys();
  bool ProcessScanCode(int scanCode, ScanCodeAction action);
  void ReleaseAll();

  void SendText(const uint8_t *text);
  void RunConsoleCommand(const char *command, const ScriptByteCode *byteCode);

  void SetScript(ButtonScriptId scriptId, const ScriptByteCode *byteCode,
                 size_t offset) {
    if (scriptId < ButtonScriptId::COUNT) {
      scriptCallbacks[(size_t)scriptId].Set(byteCode, offset);
    }
  }

  void CallPress(size_t keyIndex, uint32_t scriptTime) {
    ExecuteScriptIndex(keyIndex * 2 + 2, scriptTime);
  }
  void CallRelease(size_t keyIndex, uint32_t scriptTime) {
    ExecuteScriptIndex(keyIndex * 2 + 3, scriptTime);
  }

  void
  CancelAllCallbacksForByteCode(const Interval<const uint8_t *> &byteCodeRange);
  void
  CancelAllTimersForByteCode(const Interval<const uint8_t *> &byteCodeRange);
  void
  CancelAllCombosForByteCode(const Interval<const uint8_t *> &byteCodeRange);
};

//---------------------------------------------------------------------------

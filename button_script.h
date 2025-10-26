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
  SET_LAYER,

  USER1,
  USER2,
  USER3,
  USER4,
  USER5,
  USER6,
  USER7,

  CONSOLE_ACCESS_REQUESTED,
  FLASH_WRITE_ACCESS_REQUESTED,

  COUNT,
};

//---------------------------------------------------------------------------

class ButtonScript : public Script {
public:
  ButtonScript(const uint8_t *byteCode);

  void ExecuteInitScript(uint32_t scriptTime) {
    isInInit = true;
    ExecuteScriptIndex(0, scriptTime);
    isInInit = false;
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
  static const char *GetRelyingPartyId();
  static int GetSignatureAlgorithm();
  static void SetSignatureAlgorithms(const uint8_t *algorithms);

  void PrintEventHistory();

  void Reset();
  void SetReinit(bool value) { isInReinit = value; }

  static void RemoveScriptTimers();
  void CancelAllScriptsForByteCode(const ScriptByteCode *byteCode,
                                   size_t byteCodeSize);

  void SetScriptTime(uint32_t scriptTime) { this->scriptTime = scriptTime; }

  // Helpers.
  static size_t GetPressScriptIndex(size_t buttonIndex) {
    return 2 * buttonIndex + 2;
  }

  static size_t GetReleaseScriptIndex(size_t buttonIndex) {
    return 2 * buttonIndex + 3;
  }

  static size_t GetAnalogInputScriptIndex(size_t buttonCount,
                                          size_t analogValueIndex) {
    return GetPressScriptIndex(buttonCount) + analogValueIndex;
  }

  static size_t GetEncoderCWScriptIndex(size_t buttonCount,
                                        size_t analogValueCount,
                                        size_t encoderIndex) {
    return GetAnalogInputScriptIndex(buttonCount, analogValueCount) +
           2 * encoderIndex;
  }

  static size_t GetEncoderCCWScriptIndex(size_t buttonCount,
                                         size_t analogValueCount,
                                         size_t encoderIndex) {
    return GetAnalogInputScriptIndex(buttonCount, analogValueCount) +
           2 * encoderIndex + 1;
  }

  static size_t GetPointerScriptIndex(size_t buttonCount,
                                      size_t analogValueCount,
                                      size_t encoderCount,
                                      size_t pointerScriptIndex) {
    return GetEncoderCWScriptIndex(buttonCount, analogValueCount,
                                   encoderCount) +
           pointerScriptIndex;
  }

  class TestHelper;

private:
  class Function;
  class TimerContext;
  class NullTimerContext;

  static constexpr size_t EVENT_HISTORY_COUNT = 4;
  static void (*const FUNCTION_TABLE[])(ButtonScript &, const ScriptByteCode *);

  struct ScriptCallback {
    const ScriptByteCode *byteCode;
    size_t offset;

    void Set(const ScriptByteCode *byteCode, size_t offset) {
      this->byteCode = byteCode;
      this->offset = offset;
    }
  };

  bool isInInit;
  bool isInReinit;
  bool isScriptRgbEnabled = true;
  uint8_t inPressAllCount;
  uint8_t inReleaseAllCount;
  uint8_t formatStringWriterIndex;
  uint32_t pressCount;
  uint32_t releaseCount;
  uint32_t scriptTime;
  StenoKeyState stenoState;
  const char *eventHistory[EVENT_HISTORY_COUNT];
  ScriptCallback scriptCallbacks[(size_t)ButtonScriptId::COUNT];
  ButtonState buttonState;
  BitField<256> keyState;
  BitField<32> mouseButtonState;
  LimitedBufferWriter formatStringWriter[2];
  LimitedBufferWriter consoleWriter;
  List<void *> buffers;

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
    ExecuteScriptIndex(GetPressScriptIndex(keyIndex), scriptTime);
  }
  void CallRelease(size_t keyIndex, uint32_t scriptTime) {
    ExecuteScriptIndex(GetReleaseScriptIndex(keyIndex), scriptTime);
  }

  void
  CancelAllCallbacksForByteCode(const Interval<const uint8_t *> &byteCodeRange);
  void
  CancelAllTimersForByteCode(const Interval<const uint8_t *> &byteCodeRange);
  void
  CancelAllCombosForByteCode(const Interval<const uint8_t *> &byteCodeRange);
};

//---------------------------------------------------------------------------

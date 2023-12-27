//---------------------------------------------------------------------------

#pragma once
#include "bit_field.h"
#include "scan_code_action.h"
#include "steno_key_state.h"
#include "writer.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------
//
// Script provides a configurable way for keys to be set up.
//
//---------------------------------------------------------------------------

enum class ScriptId {
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

  COUNT,
};

//---------------------------------------------------------------------------

class Script {
public:
  Script(const uint8_t *byteCode);

  void ExecuteInitScript(uint32_t scriptTime) {
    ExecuteScriptIndex(0, scriptTime);
  }
  void ExecuteTickScript(uint32_t scriptTime) {
    ExecuteScriptIndex(1, scriptTime);
  }

  void ExecuteScriptId(ScriptId scriptId, uint32_t scriptTime);

  void HandlePress(size_t keyIndex, uint32_t scriptTime) {
    buttonState.Set(keyIndex);
    ExecuteScriptIndex(keyIndex * 2 + 2, scriptTime);
  }

  void HandleRelease(size_t keyIndex, uint32_t scriptTime) {
    buttonState.Clear(keyIndex);
    ExecuteScriptIndex(keyIndex * 2 + 3, scriptTime);
  }

  void PrintInfo() const;

  bool IsTickScriptEmpty() const { return IsScriptIndexEmpty(1); }
  bool IsScriptIndexEmpty(size_t index) const;

  static bool IsWaitingForUserPresence();
  static void ReplyUserPresence(bool present);

  void EnableScriptEvents() { scriptEventsEnabled = true; }
  void DisableScriptEvents() { scriptEventsEnabled = false; }

private:
  struct ScriptTimerContext;

  static const size_t MAX_STACK_SIZE = 256;

  bool cancelStenoState = false;
  bool scriptEventsEnabled = false;
  int inPressAllCount = 0;
  uint32_t scriptTime;
  const uint8_t *byteCode;
  intptr_t *stackTop = stack;
  StenoKeyState stenoState;
  LimitedBufferWriter consoleWriter;
  size_t scriptOffsets[(int)ScriptId::COUNT];
  BitField<256> buttonState;
  BitField<256> keyState;
  intptr_t globals[256];
  intptr_t stack[MAX_STACK_SIZE];

  void Push(intptr_t value);
  intptr_t Pop();
  const intptr_t *Pop(size_t count);
  void UnaryOp(intptr_t (*op)(intptr_t));
  void BinaryOp(intptr_t (*op)(intptr_t, intptr_t));

  void ExecuteScriptIndex(size_t index, uint32_t scriptTime);
  void ExecuteScript(size_t offset, uint32_t scriptTime);

  bool IsScriptEmpty(size_t offset) const;

  void StartTimer(intptr_t timerId, uint32_t interval, bool isRepeating,
                  size_t offset);
  void StopTimer(intptr_t timerId);

  class ExecutionContext;

  friend class ScriptTestHelper;

  void OnStenoKeyPressed();
  void OnStenoKeyReleased();
  void OnStenoStateCancelled();
  bool ProcessScanCode(int scanCode, ScanCodeAction action);

  void SendText(const uint8_t *text) const;
  bool CheckButtonState(const uint8_t *text) const;

  void RunConsoleCommand(const char *command);
  void RunGetParameterCommand(const char *parameter);

  void SetInputHint(int hint);
  void SetScript(ScriptId scriptId, size_t scriptOffset) {
    if (scriptId < ScriptId::COUNT) {
      scriptOffsets[(size_t)scriptId] = scriptOffset;
    }
  }

  static void TimerHandler(intptr_t id, void *context);
};

//---------------------------------------------------------------------------

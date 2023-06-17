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

  COUNT,
};

struct ScriptTimer {
  bool isRepeating;
  intptr_t id;
  uint32_t lastUpdateTime;
  uint32_t interval;
  size_t scriptOffset;

  int GetTriggerDelay(uint32_t currentTime) const {
    // Since time steps can be backwards in case of receiving delayed input
    // from a split pair, it's important that this returns a signed value.
    return currentTime - (lastUpdateTime + interval);
  }
  bool ShouldTrigger(uint32_t currentTime) const {
    return GetTriggerDelay(currentTime) <= 0;
  }
};

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

  void ProcessTimers(uint32_t scriptTime);

  void PrintInfo() const;

  bool HasTimers() const { return timerCount != 0; }
  size_t GetTimerCount() const { return timerCount; }
  int GetNextTimerTriggerDelay(uint32_t currentTime) const;
  bool HasOnlyRepeatingTimers() const;
  int GetTimersGCD() const;

private:
  static const size_t MAX_STACK_SIZE = 256;
  static const size_t MAXIMUM_TIMER_COUNT = 16;
  static const size_t INVALID_TIMER_INDEX = (size_t)-1;

  bool cancelStenoState = false;
  uint8_t timerCount;
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
  ScriptTimer timers[MAXIMUM_TIMER_COUNT];

  void Push(intptr_t value);
  intptr_t Pop();
  const intptr_t *Pop(size_t count);
  void UnaryOp(intptr_t (*op)(intptr_t));
  void BinaryOp(intptr_t (*op)(intptr_t, intptr_t));

  void ExecuteScriptIndex(size_t index, uint32_t scriptTime);
  void ExecuteScript(size_t offset, uint32_t scriptTime);

  size_t GetTimerIndex(intptr_t timerId) const;
  void StartTimer(intptr_t timerId, uint32_t interval, bool isRepeating,
                  size_t offset);
  void StopTimer(intptr_t timerId);
  void RemoveTimerIndex(size_t index);

  // Interception point for controlling update frequency.
  void OnTimersUpdated();

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
};

//---------------------------------------------------------------------------

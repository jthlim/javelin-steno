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

class Script {
public:
  Script(const uint8_t *byteCode);

  void ExecuteInitScript(uint32_t scriptTime) {
    this->scriptTime = scriptTime;
    ExecuteScriptIndex(0);
  }
  void ExecuteTickScript(uint32_t scriptTime) {
    this->scriptTime = scriptTime;
    ExecuteScriptIndex(1);
  }

  void HandlePress(size_t keyIndex, uint32_t scriptTime) {
    this->scriptTime = scriptTime;
    buttonState.Set(keyIndex);
    ExecuteScriptIndex(keyIndex * 2 + 2);
  }

  void HandleRelease(size_t keyIndex, uint32_t scriptTime) {
    this->scriptTime = scriptTime;
    buttonState.Clear(keyIndex);
    ExecuteScriptIndex(keyIndex * 2 + 3);
  }

private:
  static const size_t MAX_STACK_SIZE = 256;

  bool cancelStenoState = false;
  int inPressAllCount = 0;
  uint32_t scriptTime;
  const uint8_t *byteCode;
  intptr_t *stackTop = stack;
  StenoKeyState stenoState;
  LimitedBufferWriter consoleWriter;
  BitField<256> buttonState;
  BitField<256> keyState;
  intptr_t globals[256];
  intptr_t stack[MAX_STACK_SIZE];

  void Push(intptr_t value);
  intptr_t Pop();
  const intptr_t *Pop(size_t count);
  void UnaryOp(intptr_t (*op)(intptr_t));
  void BinaryOp(intptr_t (*op)(intptr_t, intptr_t));

  void ExecuteScriptIndex(size_t index);

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
};

//---------------------------------------------------------------------------

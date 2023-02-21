//---------------------------------------------------------------------------

#pragma once
#include "bit_field.h"
#include "steno_key_state.h"
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------
//
// Script provides a configurable way for keys to be set up.
//
// Up to 4x 32-bit int global variables 4x local variables can be used.
//
//---------------------------------------------------------------------------

class Script {
public:
  Script(const uint8_t *byteCode);

  void ExecuteInitScript() { ExecuteScriptIndex(0); }
  void ExecuteTickScript() { ExecuteScriptIndex(1); }

  void HandlePress(size_t keyIndex) {
    buttonState.Set(keyIndex);
    ExecuteScriptIndex(keyIndex * 2 + 2);
  }

  void HandleRelease(size_t keyIndex) {
    buttonState.Clear(keyIndex);
    ExecuteScriptIndex(keyIndex * 2 + 3);
  }

private:
  static const size_t MAX_STACK_SIZE = 32;

  bool cancelStenoState = false;
  int inPressAllCount = 0;
  const uint8_t *byteCode;
  int *stackTop = stack;
  StenoKeyState stenoState;
  int globals[64];
  BitField<256> buttonState;
  BitField<256> keyState;
  int stack[MAX_STACK_SIZE];

  void Push(int value);
  int Pop();
  void UnaryOp(int (*op)(int));
  void BinaryOp(int (*op)(int, int));

  void ExecuteScriptIndex(size_t index);

  class ExecutionContext;

  friend class ScriptTestHelper;

  void OnStenoKeyPressed();
  void OnStenoKeyReleased();
  void OnStenoStateCancelled();

  void SendText(const uint8_t *text) const;
  bool CheckButtonState(const uint8_t *text) const;
};

//---------------------------------------------------------------------------

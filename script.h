//---------------------------------------------------------------------------

#pragma once
#include "script_byte_code.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Script {
public:
  Script(const uint8_t *byteCode, void (*const *functionTable)(Script &))
      : byteCode((const ScriptByteCode *)byteCode),
        functionTable(functionTable) {}

  void Push(intptr_t value) {
    assert(stackTop < stack + MAX_STACK_SIZE);
    *stackTop++ = value;
  }

  intptr_t Pop() {
    assert(stackTop > stack);
    // Expanded for codegen.
    // return *--stackTop;
    intptr_t *v = stackTop;
    const intptr_t result = *--v;
    stackTop = v;
    return result;
  }

  void PrintScriptGlobals() const;

  bool IsScriptEmpty(size_t offset) const;
  bool IsScriptIndexEmpty(size_t index) const {
    return IsScriptEmpty(byteCode->scriptOffsets[index]);
  }

  void ExecuteScript(size_t offset);
  void ExecuteScriptIndex(size_t index) {
    ExecuteScript(byteCode->scriptOffsets[index]);
  }

  void ExecuteScript(size_t offset, const intptr_t *parameters,
                     size_t parameterCount);
  void ExecuteScriptIndex(size_t index, const intptr_t *parameters,
                          size_t parameterCount) {
    ExecuteScript(byteCode->scriptOffsets[index], parameters, parameterCount);
  }

  void Reset();
  bool IsValid() const { return byteCode->IsValid(); }

  uint32_t Crc() const { return byteCode->Crc(); }

  intptr_t *GetStackTop() const { return stackTop; }
  void SetStackTop(intptr_t *p) { stackTop = p; }

  void SetGlobal(size_t i, intptr_t v) { globals[i] = v; }
  intptr_t GetGlobal(size_t i) const { return globals[i]; }

  const uint8_t *FindStringOrReturnOriginal(const uint8_t *string) const {
    return byteCode->FindStringOrReturnOriginal(string);
  }

protected:
  template <typename T> const T *GetScriptData(size_t offset) {
    return (const T *)(intptr_t(byteCode) + offset);
  }
  template <typename T> void PushDataOffset(const T *data) {
    Push(intptr_t(data) - intptr_t(byteCode));
  }

  // Executes at offset with no stack and offset checks.
  void Run(size_t offset);

private:
  class StackPointer;
  class ProgramCounter;

  static const size_t MAX_STACK_SIZE = 256;

  const ScriptByteCode *const byteCode;
  intptr_t *stackTop = stack;
  void (*const *const functionTable)(Script &);
  intptr_t globals[256];
  intptr_t stack[MAX_STACK_SIZE];
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "script_byte_code.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Script {
public:
  Script(const uint8_t *byteCode,
         void (*const *functionTable)(Script &, const ScriptByteCode *byteCode))
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
  void ExecuteScript(const ScriptByteCode *byteCode, size_t offset);
  void ExecuteScriptIndex(size_t index) {
    ExecuteScript(byteCode->scriptOffsets[index]);
  }
  void ExecuteByteCode(const ScriptByteCode *code) {
    Run(code->scriptOffsets[0], code);
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

  intptr_t GetGlobal(size_t i) const { return globals[i]; }
  void SetGlobal(size_t i, intptr_t v) { globals[i] = v; }

protected:
  // Executes at offset with no stack and offset checks.
  void Run(size_t offset, const ScriptByteCode *byteCode);
  void Run(size_t offset) { Run(offset, byteCode); }

  size_t GetDataOffset(const void *p) const {
    return byteCode->GetDataOffset(p);
  }
  const uint8_t *GetInstructionsAtOffset(size_t offset) const {
    return byteCode->GetScriptData<uint8_t>(offset);
  }

private:
  class StackPointer;
  class ProgramCounter;

  static constexpr size_t MAX_STACK_SIZE = 256;

  const ScriptByteCode *const byteCode;
  intptr_t *stackTop = stack;
  void (*const *const functionTable)(Script &, const ScriptByteCode *byteCode);
  intptr_t globals[256];
  intptr_t stack[MAX_STACK_SIZE];
};

//---------------------------------------------------------------------------

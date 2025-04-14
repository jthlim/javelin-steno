//---------------------------------------------------------------------------

#include "script.h"
#include "console.h"
#include "mem.h"
#include "varint_writer.h"

#include <assert.h>

//---------------------------------------------------------------------------

void Script::Reset() {
  stackTop = stack;
  Mem::Clear(globals);
}

bool Script::IsScriptEmpty(size_t offset) const {
  return offset == 0 ||
         ((uint8_t *)byteCode)[offset] == StenoScriptByteCode::RETURN;
}

void Script::ExecuteScript(size_t offset) { ExecuteScript(byteCode, offset); }

void Script::ExecuteScript(const ScriptByteCode *byteCode, size_t offset) {
  if (offset == 0) {
    return;
  }

  intptr_t *const start = stackTop;
  Run(offset, byteCode);
  assert(stackTop == start);
}

void Script::ExecuteScript(size_t offset, const intptr_t *parameters,
                           size_t parameterCount) {
  if (offset == 0) {
    return;
  }

  intptr_t *const start = stackTop;
  for (size_t i = 0; i < parameterCount; ++i) {
    Push(parameters[i]);
  }
  Run(offset);
  assert(stackTop == start);
}

void Script::PrintScriptGlobals() const {
  char buffer[256 * 5 + 4];
  VarintWriter writer(buffer);

  int i = 0;
  while (i < 256) {
    int j = i;
    while (j < 256 && globals[j] == 0) {
      ++j;
    }
    writer.Write(j - i);
    i = j;

    while (j < 256 && globals[j] != 0) {
      ++j;
    }
    writer.Write(j - i);
    for (; i < j; ++i) {
      writer.Write(VarintWriter::ZigZagEncode((int32_t)globals[i]));
    }
  }

  Console::Printf("%D\n\n", buffer, size_t(writer.p - buffer));
}

//---------------------------------------------------------------------------

// This local class optimizes access to the stack, and places the pointer
// in a register, which dramatically reduces the memory accesses when
// executing a script.
class Script::StackPointer {
public:
  StackPointer(Script &script) { SetStackTop(script.stackTop); }

  void WriteBack(Script &script) { script.stackTop = GetStackTop(); }
  void Load(Script &script) { SetStackTop(script.stackTop); }

  intptr_t *GetStackTop() const { return p; }
  void SetStackTop(intptr_t *stackTop) { p = stackTop; }

  intptr_t Pop() { return *--p; }
  void Push(intptr_t v) { *p++ = v; }
  intptr_t Peek() { return p[-1]; }

  void UnaryOp(intptr_t (*op)(intptr_t)) { Push(op(Pop())); }

  template <typename T> void TwoParam(T op) {
#if JAVELIN_CPU_CORTEX_M0
    intptr_t a, b;
    asm("sub %2, #8\n\t"
        "ldr %0, [%2]\n\t"
        "ldr %1, [%2, #4]\n\t"
        : "=l"(a), "=l"(b), "+l"(p));
#else
    const intptr_t b = Pop();
    const intptr_t a = Pop();
#endif
    op(a, b);
  }

  template <typename T> void BinaryOp(T op) {
#if JAVELIN_CPU_CORTEX_M0
    intptr_t a, b;
    asm("sub %2, #8\n\t"
        "ldr %0, [%2]\n\t"
        "ldr %1, [%2, #4]\n\t"
        : "=l"(a), "=l"(b), "+l"(p));
#else
    const intptr_t b = Pop();
    const intptr_t a = Pop();
#endif
    Push(op(a, b));
  }

private:
  intptr_t *p;
};

class Script::ProgramCounter {
public:
  ProgramCounter(const uint8_t *p) : p(p) {}

  uint32_t ReadU8() { return *p++; }
  void Advance(intptr_t offset) { p += offset; }

  uint32_t GetU16() const { return p[0] + (p[1] << 8); }

  uint32_t ReadU16() {
    const uint32_t v = p[0] + (p[1] << 8);
    p += 2;
    return v;
  }

  int32_t ReadS16() {
    int32_t value = p[0] + (p[1] << 8);
    p += 2;
    value <<= 16;
    value >>= 16;
    return value;
  }

  int32_t ReadS24() {
    int32_t value = p[0] + (p[1] << 8) + (p[2] << 16);
    p += 3;
    value <<= 8;
    value >>= 8;
    return value;
  }

  int32_t ReadS32() {
    const int32_t v = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
    p += 4;
    return v;
  }

private:
  const uint8_t *p;
};

[[gnu::weak]] void Script::Run(size_t offset, const ScriptByteCode *bc) {
  using BC = StenoScriptByteCode;
  using OP = StenoScriptOperator;

  const uint8_t *const byteCode = (const uint8_t *)bc;

  intptr_t *locals;

  // The stack point that the code needs to pop to to exit.
  intptr_t *base = stackTop;

  // The working area for the current function
  intptr_t *frame = stackTop;
  StackPointer stack(*this);

  void (*const *const functionTable)(Script &, const ScriptByteCode *) =
      this->functionTable;
  ProgramCounter p = byteCode + offset;

#define CONTINUE goto next;

next:
  uint32_t c = p.ReadU8();

  switch (c) {
  case BC::PUSH_CONSTANT_START... BC::PUSH_CONSTANT_END:
    stack.Push(c);
    CONTINUE;
  case BC::PUSH_BYTES_1U: {
    int value = p.ReadU8();
    if (value < 0x3c) {
      value -= 0x3c;
    }
    stack.Push(value);
    CONTINUE;
  }
  case BC::PUSH_BYTES_2S:
    stack.Push(p.ReadS16());
    CONTINUE;
  case BC::PUSH_BYTES_3S:
    stack.Push(p.ReadS24());
    CONTINUE;
  case BC::PUSH_BYTES_4:
    stack.Push(p.ReadS32());
    CONTINUE;
  case BC::LOAD_GLOBAL_BEGIN... BC::LOAD_GLOBAL_END:
    stack.Push(globals[c - BC::LOAD_GLOBAL_BEGIN]);
    CONTINUE;
  case BC::LOAD_GLOBAL_VALUE: {
    const int globalIndex = p.ReadU8();
    stack.Push(globals[globalIndex]);
    CONTINUE;
  }
  case BC::LOAD_GLOBAL_INDEX: {
    const intptr_t globalBaseIndex = p.ReadU8();
    const intptr_t index = stack.Pop();
    stack.Push(globals[globalBaseIndex + index]);
    CONTINUE;
  }
  case BC::STORE_GLOBAL_BEGIN... BC::STORE_GLOBAL_END:
    globals[c - BC::STORE_GLOBAL_BEGIN] = stack.Pop();
    CONTINUE;
  case BC::STORE_GLOBAL_VALUE: {
    const int globalIndex = p.ReadU8();
    globals[globalIndex] = stack.Pop();
    CONTINUE;
  }
  case BC::STORE_GLOBAL_INDEX: {
    const int globalBaseIndex = p.ReadU8();
    stack.TwoParam([&, globalBaseIndex](intptr_t index, intptr_t value) {
      globals[globalBaseIndex + index] = value;
    });
    CONTINUE;
  }
  case BC::LOAD_LOCAL_BEGIN... BC::LOAD_LOCAL_END:
    stack.Push(locals[c - BC::LOAD_LOCAL_BEGIN]);
    CONTINUE;
  case BC::LOAD_LOCAL_VALUE: {
    const int localIndex = p.ReadU8();
    stack.Push(locals[localIndex]);
    CONTINUE;
  }
  case BC::LOAD_LOCAL_INDEX: {
    const int localBaseIndex = p.ReadU8();
    const intptr_t index = stack.Pop();
    stack.Push(locals[localBaseIndex + index]);
    CONTINUE;
  }
  case BC::STORE_LOCAL_BEGIN... BC::STORE_LOCAL_END:
    locals[c - BC::STORE_LOCAL_BEGIN] = stack.Pop();
    CONTINUE;
  case BC::STORE_LOCAL_VALUE: {
    const int localIndex = p.ReadU8();
    locals[localIndex] = stack.Pop();
    CONTINUE;
  }
  case BC::STORE_LOCAL_INDEX: {
    const int localBaseIndex = p.ReadU8();
    stack.TwoParam([=](intptr_t index, intptr_t value) {
      locals[localBaseIndex + index] = value;
    });
    CONTINUE;
  }
  case BC::OPERATOR_START + (int)OP::NOT:
    stack.UnaryOp([](intptr_t a) { return (intptr_t)!a; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::NEGATIVE:
    stack.UnaryOp([](intptr_t a) { return -a; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::MULTIPLY:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a * b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::QUOTIENT:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a / b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::REMAINDER:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a % b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::ADD:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a + b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::SUBTRACT:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a - b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::EQUALS:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a == b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::NOT_EQUALS:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a != b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::LESS_THAN:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a < b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::GREATER_THAN:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a > b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::LESS_THAN_OR_EQUAL_TO:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a <= b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::GREATER_THAN_OR_EQUAL_TO:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a >= b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::BITWISE_AND:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a & b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::BITWISE_OR:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a | b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::BITWISE_XOR:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a ^ b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::AND:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a && b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::OR:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a || b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::SHIFT_LEFT:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a << b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::ARITHMETIC_SHIFT_RIGHT:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return a >> b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::LOGICAL_SHIFT_RIGHT:
    stack.BinaryOp([](intptr_t a, intptr_t b) { return uintptr_t(a) >> b; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::BYTE_LOOKUP:
    stack.BinaryOp([=](intptr_t offset, intptr_t index) {
      const uint8_t *data = byteCode + offset;
      return data[index];
    });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::WORD_LOOKUP:
    stack.BinaryOp([=](intptr_t offset, intptr_t index) {
      const intptr_t *data = (const intptr_t *)(byteCode + offset);
      return data[index];
    });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::INCREMENT:
    stack.UnaryOp([](intptr_t a) { return a + 1; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::DECREMENT:
    stack.UnaryOp([](intptr_t a) { return a - 1; });
    CONTINUE;
  case BC::OPERATOR_START + (int)OP::HALF_WORD_LOOKUP:
    stack.BinaryOp([=](intptr_t offset, intptr_t index) {
      const uint16_t *data = (const uint16_t *)(byteCode + offset);
      return data[index];
    });
    CONTINUE;
  case BC::CALL_INTERNAL: {
    stack.WriteBack(*this);
    const uint8_t function = p.ReadU8();
    (*functionTable[function])(*this, (const ScriptByteCode *)byteCode);
    stack.Load(*this);
    CONTINUE;
  }
  case BC::CALL: {
    const size_t offset = p.ReadU16();
    stack.WriteBack(*this);
    Run(offset, (const ScriptByteCode *)byteCode);
    stack.Load(*this);
    CONTINUE;
  }
  case BC::RETURN:
  exit: {
    intptr_t *p = base;
    while (frame != stack.GetStackTop()) {
      *p++ = *frame++;
    }
    stackTop = p;
    return;
  }
  case BC::POP:
    stack.Pop();
    CONTINUE;
  case BC::ENTER_FUNCTION: {
    const size_t parameterCount = p.ReadU8();
    const size_t localsCount = p.ReadU8();

    locals = stack.GetStackTop() - parameterCount;

    // This is necessary when a bytecode optimization changes a tail call
    // to a jump -- in this case, the base should not be modified.
    if (locals < base) {
      base = locals;
    }

    frame = stack.GetStackTop() + localsCount;
    stack.SetStackTop(frame);
    CONTINUE;
  }
  case BC::CALL_VALUE: {
    const size_t offset = stack.Pop();
    if (offset != 0) {
      stack.WriteBack(*this);
      Run(offset);
      stack.Load(*this);
    }
    CONTINUE;
  }
  case BC::JUMP_VALUE: {
    const size_t offset = stack.Pop();
    if (offset == 0) {
      goto exit;
    }
    p = byteCode + offset;
    CONTINUE;
  }
  case BC::RETURN_IF_ZERO:
    if (!stack.Pop()) {
      goto exit;
    }
    CONTINUE;
  case BC::RETURN_IF_NOT_ZERO:
    if (stack.Pop()) {
      goto exit;
    }
    CONTINUE;
  case BC::DUP:
    stack.Push(stack.Peek());
    CONTINUE;
  case BC::JUMP_SHORT_BEGIN... BC::JUMP_SHORT_END: {
    const int offset = c + 1 - BC::JUMP_SHORT_BEGIN;
    p.Advance(offset);
    CONTINUE;
  }
  case BC::JUMP_LONG:
    p = byteCode + p.GetU16();
    CONTINUE;
  case BC::JUMP_IF_ZERO_SHORT_BEGIN... BC::JUMP_IF_ZERO_SHORT_END:
    if (!stack.Pop()) {
      const int offset = c + 1 - BC::JUMP_IF_ZERO_SHORT_BEGIN;
      p.Advance(offset);
    }
    CONTINUE;
  case BC::JUMP_IF_ZERO_LONG: {
    const size_t offset = p.ReadU16();
    if (!stack.Pop()) {
      p = byteCode + offset;
    }
    CONTINUE;
  }
  case BC::JUMP_IF_NOT_ZERO_SHORT_BEGIN... BC::JUMP_IF_NOT_ZERO_SHORT_END:
    if (stack.Pop()) {
      const int offset = c + 1 - BC::JUMP_IF_NOT_ZERO_SHORT_BEGIN;
      p.Advance(offset);
    }
    CONTINUE;
  case BC::JUMP_IF_NOT_ZERO_LONG: {
    const size_t offset = p.ReadU16();
    if (stack.Pop()) {
      p = byteCode + offset;
    }
    CONTINUE;
  }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

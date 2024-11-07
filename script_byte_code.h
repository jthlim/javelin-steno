//---------------------------------------------------------------------------

#pragma once
#include "crc.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

const size_t SCRIPT_BYTE_CODE_VERSION = 4;
const uint32_t SCRIPT_MAGIC = 0x3053534a + (SCRIPT_BYTE_CODE_VERSION << 24);

//---------------------------------------------------------------------------

class StenoScriptByteCode {
public:
  enum Value {
    PUSH_CONSTANT_START = 0, // 0x00-0x3f
    PUSH_CONSTANT_END = 0x3b,

    PUSH_BYTES_1U = 0x3c,
    PUSH_BYTES_2S = 0x3d,
    PUSH_BYTES_3S = 0x3e,
    PUSH_BYTES_4 = 0x3f,

    LOAD_GLOBAL_BEGIN = 0x40,
    LOAD_GLOBAL_END = 0x45,
    LOAD_GLOBAL_VALUE = 0x46,
    LOAD_GLOBAL_INDEX = 0x47,

    STORE_GLOBAL_BEGIN = 0x48,
    STORE_GLOBAL_END = 0x4d,
    STORE_GLOBAL_VALUE = 0x4e,
    STORE_GLOBAL_INDEX = 0x4f,

    LOAD_LOCAL_BEGIN = 0x50,
    LOAD_LOCAL_END = 0x5d,
    LOAD_LOCAL_VALUE = 0x5e,
    LOAD_LOCAL_INDEX = 0x5f,

    STORE_LOCAL_BEGIN = 0x60,
    STORE_LOCAL_END = 0x6d,
    STORE_LOCAL_VALUE = 0x6e,
    STORE_LOCAL_INDEX = 0x6f,

    OPERATOR_START = 0x70,
    OPERATOR_END = 0x8f,

    CALL_INTERNAL = 0x90,
    CALL = 0x91,
    RETURN = 0x92,
    POP = 0x93,
    ENTER_FUNCTION = 0x94,
    CALL_VALUE = 0x95,
    JUMP_VALUE = 0x96,
    RETURN_IF_ZERO = 0x97,
    RETURN_IF_NOT_ZERO = 0x98,

    JUMP_SHORT_BEGIN = 0xa0,
    JUMP_SHORT_END = 0xbe,
    JUMP_LONG = 0xbf,

    JUMP_IF_ZERO_SHORT_BEGIN = 0xc0,
    JUMP_IF_ZERO_SHORT_END = 0xde,
    JUMP_IF_ZERO_LONG = 0xdf,

    JUMP_IF_NOT_ZERO_SHORT_BEGIN = 0xe0,
    JUMP_IF_NOT_ZERO_SHORT_END = 0xfe,
    JUMP_IF_NOT_ZERO_LONG = 0xff,
  };
};

enum class StenoScriptOperator : uint8_t {
  NOT,                      // 0
  NEGATIVE,                 // 1
  MULTIPLY,                 // 2
  QUOTIENT,                 // 3
  REMAINDER,                // 4
  ADD,                      // 5
  SUBTRACT,                 // 6
  EQUALS,                   // 7
  NOT_EQUALS,               // 8
  LESS_THAN,                // 9
  LESS_THAN_OR_EQUAL_TO,    // 0xa
  GREATER_THAN,             // 0xb
  GREATER_THAN_OR_EQUAL_TO, // 0xc
  BITWISE_AND,              // 0xd
  BITWISE_OR,               // 0xe
  BITWISE_XOR,              // 0xf
  AND,                      // 0x10
  OR,                       // 0x11
  SHIFT_LEFT,               // 0x12
  ARITHMETIC_SHIFT_RIGHT,   // 0x13
  LOGICAL_SHIFT_RIGHT,      // 0x14
  BYTE_LOOKUP,              // 0x15
  WORD_LOOKUP,              // 0x16
  INCREMENT,                // 0x17
  DECREMENT,                // 0x18
  HALF_WORD_LOOKUP,         // 0x19
};

struct StenoScriptHashTable {
  uint16_t size;
  uint16_t offsets[1];
};

struct ScriptByteCode {
  union {
    uint8_t magic[4]; // JSS4
    uint32_t magic4;
  };
  uint16_t stringHashTableOffset;
  uint16_t scriptOffsets[0];

  bool IsValid() const { return magic4 == SCRIPT_MAGIC; }
  const uint8_t *FindStringOrReturnOriginal(const uint8_t *string) const;
  const StenoScriptHashTable *GetHashTable() const {
    const uint8_t *base = (const uint8_t *)this;
    return (const StenoScriptHashTable *)(base + stringHashTableOffset);
  }

  size_t GetLength() const {
    return stringHashTableOffset + 2 + sizeof(uint16_t) * GetHashTable()->size;
  }

  uint32_t Crc() const { return Crc32(this, GetLength()); }
};

//---------------------------------------------------------------------------

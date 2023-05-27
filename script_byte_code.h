//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

const size_t SCRIPT_BYTE_CODE_REVISION = 2;

class StenoScriptByteCode {
public:
  enum Value {
    PUSH_CONSTANT_START = 0, // 0x00-0x3f
    PUSH_CONSTANT_END = 0x3f,

    OPERATOR_START = 0x40, // 0x40-0x5f
    OPERATOR_END = 0x5f,

    JUMP_SHORT_START = 0x60, // 0x60-0x7f - 1-32 relative offset
    JUMP_SHORT_END = 0x7f,

    JUMP_IF_ZERO_SHORT_START = 0x80, // 0x80-0x9f - 1-32 relative offset
    JUMP_IF_ZERO_SHORT_END = 0x9f,

    JUMP_IF_NOT_ZERO_SHORT_START = 0xa0, // 0xa0-0xbf - 1-32 relative offset
    JUMP_IF_NOT_ZERO_SHORT_END = 0xbf,

    PUSH_BYTES_1U = 0xc0,
    PUSH_BYTES_2S = 0xc1,
    PUSH_BYTES_3S = 0xc2,
    PUSH_BYTES_4 = 0xc3,

    RETURN = 0xc4,                // 0xc4
    CALL = 0xc5,                  // 0xc5
    JUMP_LONG = 0xc6,             // 0xc6 - 2 byte absolute offset
    JUMP_IF_ZERO_LONG = 0xc7,     // 0xc7 - 2 byte absolute offset
    JUMP_IF_NOT_ZERO_LONG = 0xc8, // 0xc8 - 2 byte absolute offset
    POP = 0xc9,

    GLOBAL_LOAD = 0xca,
    GLOBAL_STORE = 0xcb,

    EXTENDED_CALL_FUNCTION = 0xcc,

    GLOBAL_LOAD_INDEX = 0xcd,
    GLOBAL_STORE_INDEX = 0xce,

    PARAM_STORE = 0xcf,

    PARAM_LOAD_START = 0xd0, // 0xd0-0xd7
    PARAM_LOAD_END = 0xd7,
    PARAM_STORE_COUNT_START = 0xd8,
    PARAM_STORE_COUNT_END = 0xdf,

    LOCAL_LOAD_START = 0xe0, // 0xe0-0xe3
    LOCAL_LOAD_END = 0xe3,
    LOCAL_STORE_START = 0xe4, // 0xe4-0xe7
    LOCAL_STORE_END = 0xe7,

    GLOBAL_LOAD_START = 0xe8, // 0xe8-0xeb
    GLOBAL_LOAD_END = 0xeb,
    GLOBAL_STORE_START = 0xec, // 0xec-0xef
    GLOBAL_STORE_END = 0xef,

    CALL_FUNCTION_START = 0xf0, // 0xf0-0xff
    CALL_FUNCTION_END = 0xff,
  };
};

enum class StenoScriptFunction : uint8_t {
  PRESS_SCAN_CODE,
  RELEASE_SCAN_CODE,
  TAP_SCAN_CODE,
  IS_SCAN_CODE_PRESSED,
  PRESS_STENO_KEY,
  RELEASE_STENO_KEY,
  IS_STENO_KEY_PRESSED,
  RELEASE_ALL,
  IS_BUTTON_PRESSED,
  PRESS_ALL,
  SEND_TEXT,
  CONSOLE,
  CHECK_BUTTON_STATE,
  IS_IN_PRESS_ALL,
  SET_RGB,
  GET_TIME,
};

enum class StenoExtendedScriptFunction : uint8_t {
  GET_LED_STATUS,
  SET_GPIO_PIN,
  CLEAR_DISPLAY,
  SET_AUTO_DRAW,
  SET_SCREEN_ON,
  SET_SCREEN_CONTRAST,
  DRAW_PIXEL,
  DRAW_LINE,
  DRAW_IMAGE,
  DRAW_TEXT,
  SET_DRAW_COLOR,
  DRAW_RECT,
  SET_HSV,
  RAND,
  IS_USB_MOUNTED,
  IS_USB_SUSPENDED,
  GET_PARAMETER,
  IS_CONNECTED,
  GET_ACTIVE_CONNECTION,
  SET_PREFERRED_CONNECTION,
  IS_PAIR_CONNECTED,
  START_BLE_PAIRING,
  GET_BLE_PROFILE,
  SET_BLE_PROFILE,
  IS_HOST_SLEEPING,
  IS_MAIN_POWERED,
  IS_CHARGING,
  GET_BATTERY_PERCENTAGE,
  GET_ACTIVE_PAIR_CONNECTION,
  SET_BOARD_POWER,
  SEND_EVENT,
  IS_PAIR_POWERED,
  SET_INPUT_HINT,
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
};

struct StenoScriptByteCodeData {
  uint8_t magic[4]; // JSS0
  uint16_t stringHashTableOffset;
  uint16_t offsets[0];

  const uint8_t *FindStringOrReturnOriginal(const uint8_t *string) const;
};

//---------------------------------------------------------------------------

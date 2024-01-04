//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

const size_t SCRIPT_BYTE_CODE_REVISION = 3;
const uint32_t SCRIPT_MAGIC = 0x3053534a + (SCRIPT_BYTE_CODE_REVISION << 24);

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
  SET_SCRIPT,
  IS_BOARD_POWERED,
  START_TIMER,
  STOP_TIMER,
  IS_TIMER_ACTIVE,
  IS_BLE_PROFILE_CONNECTED,
  DISCONNECT_BLE,
  UNPAIR_BLE,
  IS_BLE_PROFILE_PAIRED,
  IS_BLE_PROFILE_SLEEPING,
  IS_BLE_ADVERTISING,
  IS_BLE_SCANNING,
  IS_WAITING_FOR_USER_PRESENCE,
  REPLY_USER_PRESENCE,
  SET_GPIO_INPUT_PIN,
  READ_GPIO_PIN,
  DRAW_GRAYSCALE_RANGE,
  SET_GPIO_PIN_DUTY_CYCLE,
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
  union {
    uint8_t magic[4]; // JSS3
    uint32_t magic4;
  };
  uint16_t stringHashTableOffset;
  uint16_t offsets[0];

  bool IsValid() const { return magic4 == SCRIPT_MAGIC; }
  const uint8_t *FindStringOrReturnOriginal(const uint8_t *string) const;
};

//---------------------------------------------------------------------------

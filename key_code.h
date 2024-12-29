//---------------------------------------------------------------------------
// Key codes as defined by USB spec 1.11
//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

// pico-sdk defines F# macros, that interfere with the KeyCodes
#undef F1
#undef F2
#undef F3
#undef F4
#undef F5
#undef F6
#undef F7
#undef F8
#undef F9
#undef F10
#undef F11
#undef F12
#undef F13
#undef F14
#undef F15

//---------------------------------------------------------------------------

struct KeyCode {
  enum Value : uint8_t {
    A = 0x04,
    B = 0x05,
    C = 0x06,
    D = 0x07,
    E = 0x08,
    F = 0x09,
    G = 0x0a,
    H = 0x0b,
    I = 0x0c,
    J = 0x0d,
    K = 0x0e,
    L = 0x0f,
    M = 0x10,
    N = 0x11,
    O = 0x12,
    P = 0x13,
    Q = 0x14,
    R = 0x15,
    S = 0x16,
    T = 0x17,
    U = 0x18,
    V = 0x19,
    W = 0x1a,
    X = 0x1b,
    Y = 0x1c,
    Z = 0x1d,
    _1 = 0x1e,
    _2 = 0x1f,
    _3 = 0x20,
    _4 = 0x21,
    _5 = 0x22,
    _6 = 0x23,
    _7 = 0x24,
    _8 = 0x25,
    _9 = 0x26,
    _0 = 0x27,

    ENTER = 0x28,
    ESC = 0x29,
    BACKSPACE = 0x2a,
    TAB = 0x2b,
    SPACE = 0x2c,
    MINUS = 0x2d,
    EQUAL = 0x2e,
    L_BRACKET = 0x2f,
    R_BRACKET = 0x30,
    BACKSLASH = 0x31,
    HASH_TILDE = 0x32,
    SEMICOLON = 0x33,
    APOSTROPHE = 0x34,
    GRAVE = 0x35,
    COMMA = 0x36,
    DOT = 0x37,
    SLASH = 0x38,
    CAPS_LOCK = 0x39,

    F1 = 0x3a,
    F2 = 0x3b,
    F3 = 0x3c,
    F4 = 0x3d,
    F5 = 0x3e,
    F6 = 0x3f,
    F7 = 0x40,
    F8 = 0x41,
    F9 = 0x42,
    F10 = 0x43,
    F11 = 0x44,
    F12 = 0x45,

    PRINT_SCREEN = 0x46,
    SCROLL_LOCK = 0x47,
    PAUSE = 0x48,
    INSERT = 0x49,
    HOME = 0x4a,
    PAGE_UP = 0x4b,
    DELETE = 0x4c,
    END = 0x4d,
    PAGE_DOWN = 0x4e,
    RIGHT = 0x4f,
    LEFT = 0x50,
    DOWN = 0x51,
    UP = 0x52,

    NUM_LOCK = 0x53,
    KP_SLASH = 0x54,
    KP_ASTERISK = 0x55,
    KP_MINUS = 0x56,
    KP_PLUS = 0x57,
    KP_ENTER = 0x58,
    KP_1 = 0x59,
    KP_2 = 0x5a,
    KP_3 = 0x5b,
    KP_4 = 0x5c,
    KP_5 = 0x5d,
    KP_6 = 0x5e,
    KP_7 = 0x5f,
    KP_8 = 0x60,
    KP_9 = 0x61,
    KP_0 = 0x62,
    KP_DOT = 0x63,

    BACKSLASH_PIPE = 0x64,
    COMPOSE = 0x65,
    POWER = 0x66,
    KP_EQUAL = 0x67,

    F13 = 0x68,
    F14 = 0x69,
    F15 = 0x6a,
    F16 = 0x6b,
    F17 = 0x6c,
    F18 = 0x6d,
    F19 = 0x6e,
    F20 = 0x6f,
    F21 = 0x70,
    F22 = 0x71,
    F23 = 0x72,
    F24 = 0x73,

    OPEN = 0x74,
    HELP = 0x75,
    MENU = 0x76,
    SELECT = 0x77,
    STOP = 0x78,
    AGAIN = 0x79,
    UNDO = 0x7a,
    CUT = 0x7b,
    COPY = 0x7c,
    PASTE = 0x7d,
    FIND = 0x7e,
    MUTE = 0x7f,
    VOLUME_UP = 0x80,
    VOLUME_DOWN = 0x81,

    KP_COMMA = 0x85,
    RO = 0x87,
    KANA = 0x88,
    YEN = 0x89,
    HENKAN = 0x8a,
    MUHENKAN = 0x8b,
    KP_JP_COMMA = 0x8c,

    HANGEUL = 0x90,
    HANJA = 0x91,
    KATAKANA = 0x92,
    HIRAGANA = 0x93,
    HANKAKU_ZENKAKU = 0x94,
    FURIGANA = 0x95,

    CLEAR = 0x9c,

    L_CTRL = 0xe0,
    L_SHIFT = 0xe1,
    L_ALT = 0xe2,
    L_META = 0xe3,
    R_CTRL = 0xe4,
    R_SHIFT = 0xe5,
    R_ALT = 0xe6,
    R_META = 0xe7,

    // From Consumer Page (0x0c)
    CONSUMER_PAGE_PLAY = 0xa0,                // UsageId 0xb0
    CONSUMER_PAGE_PAUSE = 0xa1,               // UsageId 0xb1
    CONSUMER_PAGE_RECORD = 0xa2,              // UsageId 0xb2
    CONSUMER_PAGE_FAST_FORWARD = 0xa3,        // UsageId 0xb3
    CONSUMER_PAGE_REWIND = 0xa4,              // UsageId 0xb4
    CONSUMER_PAGE_SCAN_NEXT_TRACK = 0xa5,     // UsageId 0xb5
    CONSUMER_PAGE_SCAN_PREVIOUS_TRACK = 0xa6, // UsageId 0xb6
    CONSUMER_PAGE_STOP = 0xa7,                // UsageId 0xb7
    CONSUMER_PAGE_EJECT = 0xa8,               // UsageId 0xb8
    CONSUMER_PAGE_STOP_EJECT = 0xbc,          // UsageId 0xcc
    CONSUMER_PAGE_PLAY_PAUSE = 0xbd,          // UsageId 0xcd
    CONSUMER_PAGE_PLAY_SKIP = 0xbe,           // UsageId 0xce
    CONSUMER_PAGE_MUTE = 0xd2,                // UsageId 0xe2
    CONSUMER_PAGE_VOLUME_UP = 0xd9,           // UsageId 0xe9
    CONSUMER_PAGE_VOLUME_DOWN = 0xda,         // UsageId 0xea
  };

  KeyCode() = default;
  constexpr KeyCode(uint32_t value) : value(value) {}

  bool IsVisible() const {
    return (A <= value && value <= ENTER) || (TAB <= value && value <= SLASH) ||
           (KP_SLASH <= value && value <= BACKSLASH_PIPE);
  }

  bool IsModifier() const { return L_CTRL <= value && value <= R_META; }

  // keyCode is in the lower 8 bits.
  // modifiers are in the next 8 bits and uses the values defined in
  // steno_key_code.h.
  //
  // Returns 0 if there's no simple conversion.
  static uint32_t ConvertToUnicode(uint32_t keyCodeAndModifiers);

  KeyCode TranslateForHostLayout() const;

  bool operator==(const KeyCode &other) const = default;

  uint32_t value;
};

//---------------------------------------------------------------------------

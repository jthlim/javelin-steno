//---------------------------------------------------------------------------
// Key codes as defined by USB spec 1.11
//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

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

struct KeyCode {
  enum Value {
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
    CAPS = 0x39,

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

    SYS_RQ = 0x46,
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

    KP_LEFT_PAREN = 0xb6,
    KP_RIGHT_PAREN = 0xb7,

    L_CTRL = 0xe0,
    L_SHIFT = 0xe1,
    L_ALT = 0xe2,
    L_META = 0xe3,
    R_CTRL = 0xe4,
    R_SHIFT = 0xe5,
    R_ALT = 0xe6,
    R_META = 0xe7,

    MEDIA_PLAY_PAUSE = 0xe8,
    MEDIA_STOP_CD = 0xe9,
    MEDIA_PREVIOUS_SONG = 0xea,
    MEDIA_NEXT_SONG = 0xeb,
    MEDIA_EJECT_CD = 0xec,
    MEDIA_VOLUME_UP = 0xed,
    MEDIA_VOLUME_DOWN = 0xee,
    MEDIA_MUTE = 0xef,
    MEDIA_WWW = 0xf0,
    MEDIA_BACK = 0xf1,
    MEDIA_FORWARD = 0xf2,
    MEDIA_STOP = 0xf3,
    MEDIA_FIND = 0xf4,
    MEDIA_SCROLL_UP = 0xf5,
    MEDIA_SCROLL_DOWN = 0xf6,
    MEDIA_EDIT = 0xf7,
    MEDIA_SLEEP = 0xf8,
    MEDIA_COFFEE = 0xf9,
    MEDIA_REFRESH = 0xfa,
    MEDIA_CALC = 0xfb,
  };
};

#if RUN_TESTS
#include <vector>
#endif

class Key {
public:
  struct HistoryEntry {
    HistoryEntry() = default;
    HistoryEntry(uint8_t code, bool isPress)
        : code((KeyCode::Value)code), isPress(isPress) {}

    KeyCode::Value code;
    bool isPress;
  };

  static void Press(uint8_t key);
  static void Release(uint8_t key);

  static bool IsNumLockOn();

  static void EnableHistory() { historyEnabled = true; }
  static void DisableHistory() { historyEnabled = false; }

  static bool historyEnabled;

#if RUN_TESTS
  static std::vector<HistoryEntry> history;
#endif
};

//---------------------------------------------------------------------------

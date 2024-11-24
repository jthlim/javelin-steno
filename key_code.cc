//---------------------------------------------------------------------------

#include "key_code.h"
#include "host_layout.h"
#include "steno_key_code.h"
#include <stdint.h>

//---------------------------------------------------------------------------

constexpr uint8_t KEY_CODE_TO_UNICODE[256][2] = {
    {},         // 0
    {},         // 1
    {},         // 2
    {},         // 3
    {'a', 'A'}, // 4
    {'b', 'B'}, //
    {'c', 'C'}, //
    {'d', 'D'}, //
    {'e', 'E'}, //
    {'f', 'F'}, //
    {'g', 'G'}, //
    {'h', 'H'}, //
    {'i', 'I'}, //
    {'j', 'J'}, //
    {'k', 'K'}, //
    {'l', 'L'}, //
    {'m', 'M'}, //
    {'n', 'N'}, //
    {'o', 'O'}, //
    {'p', 'P'}, //
    {'q', 'Q'}, //
    {'r', 'R'}, //
    {'s', 'S'}, //
    {'t', 'T'}, //
    {'u', 'U'}, //
    {'v', 'V'}, //
    {'w', 'W'}, //
    {'x', 'X'}, //
    {'y', 'Y'}, //
    {'z', 'Z'}, //
    {'1', '!'}, //
    {'2', '@'}, //
    {'3', '#'}, //
    {'4', '$'}, //
    {'5', '%'}, //
    {'6', '^'}, //
    {'7', '&'}, //
    {'8', '*'}, //
    {'9', '('}, //
    {'0', ')'}, //

    {'\n', '\n'}, // ENTER
    {},           // ESC
    {'\b', '\b'}, // BACKSPACE
    {'\t', 0},    // TAB
    {' ', ' '},   // SPACE
    {'-', '_'},   // MINUS
    {'=', '+'},   // EQUAL
    {'[', '{'},   // L_BRACKET
    {']', '}'},   // R_BRACKET
    {'\\', '|'},  // BACKSLASH
    {'#', '~'},   // HASH_TILDE
    {';', ':'},   // SEMICOLON
    {'\'', '"'},  // APOSTROPHE
    {'`', '~'},   // GRAVE
    {',', '<'},   // COMMA
    {'.', '>'},   // DOT
    {'/', '?'},   // SLASH
    {},           // CAPS_LOCK

    {}, // F1
    {}, // F2
    {}, // F3
    {}, // F4
    {}, // F5
    {}, // F6
    {}, // F7
    {}, // F8
    {}, // F9
    {}, // F10
    {}, // F11
    {}, // F12

    {}, // PRINT_SCREEN
    {}, // SCROLL_LOCK
    {}, // PAUSE
    {}, // INSERT
    {}, // HOME
    {}, // PAGE_UP
    {}, // DELETE
    {}, // END
    {}, // PAGE_DOWN
    {}, // RIGHT
    {}, // LEFT
    {}, // DOWN
    {}, // UP

    {},           // NUM_LOCK
    {'/', '/'},   // KP_SLASH
    {'*', '*'},   // KP_ASTERISK
    {'-', '-'},   // KP_MINUS
    {'+', '+'},   // KP_PLUS
    {'\n', '\n'}, // KP_ENTER
    {'1', '1'},   // KP_1
    {'2', '2'},   // KP_2
    {'3', '3'},   // KP_3
    {'4', '4'},   // KP_4
    {'5', '5'},   // KP_5
    {'6', '6'},   // KP_6
    {'7', '7'},   // KP_7
    {'8', '8'},   // KP_8
    {'9', '9'},   // KP_9
    {'0', '0'},   // KP_0
    {'.', '.'},   // KP_DOT

    {'\\', '|'}, // BACKSLASH_PIPE
    {},          // COMPOSE
    {},          // POWER
    {'=', '='},  // KP_EQUAL
};

//---------------------------------------------------------------------------

uint32_t KeyCode::ConvertToUnicode(uint32_t keyCodeAndModifiers) {
  if (keyCodeAndModifiers & (MODIFIER_MASK & ~MODIFIER_SHIFT_FLAG)) {
    // If there's a non-shift modifier held, don't try and convert to unicode.
    return 0;
  }

  const uint32_t hostScanCode =
      (keyCodeAndModifiers & MODIFIER_SHIFT_FLAG)
          ? MODIFIER_L_SHIFT_FLAG | (keyCodeAndModifiers & 0xff)
          : (keyCodeAndModifiers & 0xff);
  const uint32_t hostUnicode =
      HostLayouts::GetActiveLayout().GetUnicodeForScancode(hostScanCode);
  if (hostUnicode != 0) {
    return hostUnicode;
  }

  const uint32_t caseIndex =
      (keyCodeAndModifiers & MODIFIER_SHIFT_FLAG) ? 1 : 0;
  const uint32_t keyCodeIndex = keyCodeAndModifiers & 0xff;

  return KEY_CODE_TO_UNICODE[keyCodeIndex][caseIndex];
}

//---------------------------------------------------------------------------

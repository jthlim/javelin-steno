//---------------------------------------------------------------------------

#include "key_code.h"
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
};

//---------------------------------------------------------------------------

uint32_t KeyCode::ConvertToUnicode(uint32_t keyCodeAndModifiers) {
  if (keyCodeAndModifiers & (MODIFIER_MASK & ~MODIFIER_SHIFT_FLAG)) {
    // If there's a non-shift modifier held, don't try and convert to unicode.
    return 0;
  }
  uint32_t caseIndex = (keyCodeAndModifiers & MODIFIER_SHIFT_FLAG) ? 1 : 0;
  uint32_t keyCodeIndex = keyCodeAndModifiers & 0xff;

  return KEY_CODE_TO_UNICODE[keyCodeIndex][caseIndex];
}

//---------------------------------------------------------------------------

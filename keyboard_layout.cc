//---------------------------------------------------------------------------

#include "keyboard_layout.h"
#include "console.h"
#include "key_code.h"
#include "str.h"

//---------------------------------------------------------------------------

KeyboardLayout::ActiveInstance KeyboardLayout::activeInstance = {
    .layout = KeyboardLayout::QWERTY,
    .layoutTable = nullptr,
};

//---------------------------------------------------------------------------

const char *const KeyboardLayout::NAMES[] = {
    "qwerty", "dvorak", "colemak", "workman", "qgmlwy",
};

// clang-format off
const KeyboardLayoutTable DVORAK_LAYOUT_TABLE = {
  .values = {
    0, 1, 2, 3, // No translation
    KeyCode::A, KeyCode::N, KeyCode::I, KeyCode::H, // A-D
    KeyCode::D, KeyCode::Y, KeyCode::U, KeyCode::J, // E-H
    KeyCode::G, KeyCode::C, KeyCode::V, KeyCode::P, // I-L
    KeyCode::M, KeyCode::L, KeyCode::S, KeyCode::R, // M-P
    KeyCode::X, KeyCode::O, KeyCode::SEMICOLON, KeyCode::K, // Q-T
    KeyCode::F, KeyCode::DOT, KeyCode::COMMA, KeyCode::B, // U-X
    KeyCode::T, KeyCode::SLASH, 30, 31, // Y-Z, 1-2
    32, 33, 34, 35, 36, 37, 38, 39, // 3-9, 0
    40, 41, 42, 43,
    44, KeyCode::APOSTROPHE, KeyCode::R_BRACKET, KeyCode::MINUS, // minus -> apos, equal -> rbracket, lbracket -> minus
    KeyCode::EQUAL, 49, 50, KeyCode::Z, // RBracket -> Equal, Semicolon -> Z
    KeyCode::Q, 53, KeyCode::W, KeyCode::E, // apos -> Q, dot -> W, dot -> E
    KeyCode::L_BRACKET, 57, 58, 59, // slash -> lbracket
    60, 61, 62, 63,
  }
};

const KeyboardLayoutTable COLEMAK_LAYOUT_TABLE = {
  .values = {
    0, 1, 2, 3,
    KeyCode::A, KeyCode::B, KeyCode::C, KeyCode::G, // A-D
    KeyCode::K, KeyCode::E, KeyCode::T, KeyCode::H, // E-H
    KeyCode::L, KeyCode::Y, KeyCode::N, KeyCode::U, // I-L
    KeyCode::M, KeyCode::J, KeyCode::SEMICOLON, KeyCode::R, // M-P
    KeyCode::Q, KeyCode::S, KeyCode::D, KeyCode::F, // Q-T
    KeyCode::I, KeyCode::V, KeyCode::W, KeyCode::X, // U-X
    KeyCode::O, KeyCode::Z, 30, 31, // Y-Z, 1-2
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, KeyCode::P, 52, 53, 54, 55, // Semicolon -> P
    56, 57, 58, 59, 60, 61, 62, 63,
  }
};

const KeyboardLayoutTable WORKMAN_LAYOUT_TABLE = {
  .values = {
    0, 1, 2, 3,
    KeyCode::A, KeyCode::T, KeyCode::V, KeyCode::W, // A-D
    KeyCode::K, KeyCode::U, KeyCode::G, KeyCode::D, // E-H
    KeyCode::SEMICOLON, KeyCode::Y, KeyCode::N, KeyCode::M, // I-L
    KeyCode::C, KeyCode::J, KeyCode::L, KeyCode::O, // M-P
    KeyCode::Q, KeyCode::E, KeyCode::S, KeyCode::F, // Q-T
    KeyCode::I, KeyCode::B, KeyCode::R, KeyCode::X, // U-X
    KeyCode::H, KeyCode::Z, 30, 31, // Y-Z, 1-2
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, KeyCode::P, 52, 53, 54, 55, // Semicolon -> P
    56, 57, 58, 59, 60, 61, 62, 63,
  }
};

const KeyboardLayoutTable QGMLWY_LAYOUT_TABLE = {
  .values = {
    0, 1, 2, 3,
    KeyCode::J, KeyCode::O, KeyCode::C, KeyCode::A, // A-D
    KeyCode::K, KeyCode::U, KeyCode::W, KeyCode::SEMICOLON, // E-H
    KeyCode::H, KeyCode::B, KeyCode::N, KeyCode::R, // I-L
    KeyCode::E, KeyCode::F, KeyCode::L, KeyCode::M, // M-P
    KeyCode::Q, KeyCode::G, KeyCode::S, KeyCode::D, // Q-T
    KeyCode::I, KeyCode::V, KeyCode::T, KeyCode::X, // U-X
    KeyCode::Y, KeyCode::Z, 30, 31, // Y-Z, 1-2
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, KeyCode::P, 52, 53, 54, 55, // Semicolon -> P
    56, 57, 58, 59, 60, 61, 62, 63,
  }
};
// clang-format on

const KeyboardLayoutTable *const KeyboardLayout::LAYOUT_TABLES[] = {
    nullptr,
    &DVORAK_LAYOUT_TABLE,
    &COLEMAK_LAYOUT_TABLE,
    &WORKMAN_LAYOUT_TABLE,
    &QGMLWY_LAYOUT_TABLE,
};

//---------------------------------------------------------------------------

bool KeyboardLayout::SetActiveLayout(const char *name) {
  for (size_t i = 0; i < COUNT; ++i) {
    if (Str::Eq(name, NAMES[i])) {
      SetActiveLayout(Value(i));
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------

void KeyboardLayout::SetKeyboardLayout_Binding(void *context,
                                               const char *commandLine) {
  const char *keyboardLayout = strchr(commandLine, ' ');
  if (!keyboardLayout) {
    Console::Printf("ERR No keyboard layout specified\n\n");
    return;
  }
  ++keyboardLayout;

  if (SetActiveLayout(keyboardLayout)) {
    Console::SendOk();
  } else {
    Console::Printf("ERR Unable to set keyboard layout: \"%s\"\n\n",
                    keyboardLayout);
  }
}

//---------------------------------------------------------------------------

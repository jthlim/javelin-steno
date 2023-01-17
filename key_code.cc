//---------------------------------------------------------------------------

#include "key_code.h"
#include "str.h"

//---------------------------------------------------------------------------

const uint8_t *Key::layoutTable = nullptr;
bool Key::isNumLockOn = false;

//---------------------------------------------------------------------------

#if RUN_TESTS

bool Key::historyEnabled = true;
std::vector<Key::HistoryEntry> Key::history;

void Key::PressRaw(uint8_t key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, true));
}

void Key::ReleaseRaw(uint8_t key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, false));
}

#endif // RUN_TESTS

//---------------------------------------------------------------------------

const char *const KEYBOARD_LAYOUT_NAMES[] = {
    "qwerty",
    "dvorak",
    "colemak",
    "workman",
};

// clang-format off
const uint8_t DVORAK_LAYOUT_TABLE[256] = {
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
  64, 65, 66, 67, 68, 69, 70, 71,
  72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87,
  88, 89, 90, 91, 92, 93, 94, 95,
  96, 97, 98, 99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119,
  120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167,
  168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183,
  184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231,
  232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247,
  248, 249, 250, 251, 252, 253, 254, 255,
};

const uint8_t COLEMAK_LAYOUT_TABLE[256] = {
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
  48, 49, 50, 51, 52, 53, 54, 55,
  56, 57, 58, 59, 60, 61, 62, 63,
  64, 65, 66, 67, 68, 69, 70, 71,
  72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87,
  88, 89, 90, 91, 92, 93, 94, 95,
  96, 97, 98, 99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119,
  120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167,
  168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183,
  184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231,
  232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247,
  248, 249, 250, 251, 252, 253, 254, 255,
};

const uint8_t WORKMAN_LAYOUT_TABLE[256] = {
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
  48, 49, 50, KeyCode::P, 52, 53, 54, 55, // semicolon -> P
  56, 57, 58, 59, 60, 61, 62, 63,
  64, 65, 66, 67, 68, 69, 70, 71,
  72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87,
  88, 89, 90, 91, 92, 93, 94, 95,
  96, 97, 98, 99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119,
  120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167,
  168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183,
  184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231,
  232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247,
  248, 249, 250, 251, 252, 253, 254, 255,
};
// clang-format on

const uint8_t *const KEYBOARD_LAYOUT_TABLES[] = {
    nullptr,
    DVORAK_LAYOUT_TABLE,
    COLEMAK_LAYOUT_TABLE,
    WORKMAN_LAYOUT_TABLE,
};

const char *KeyboardLayoutName(KeyboardLayout layout) {
  uint8_t index = (uint8_t)layout;
  if (index > (uint8_t)KeyboardLayout::COUNT) {
    index = 0;
  }
  return KEYBOARD_LAYOUT_NAMES[index];
}

KeyboardLayout Key::GetKeyboardLayout() {
  for (size_t i = 0; i < (size_t)KeyboardLayout::COUNT; ++i) {
    if (layoutTable == KEYBOARD_LAYOUT_TABLES[i]) {
      return (KeyboardLayout)i;
    }
  }
  return KeyboardLayout::QWERTY;
}

bool Key::SetKeyboardLayout(const char *name) {
  for (size_t i = 0; i < (size_t)KeyboardLayout::COUNT; ++i) {
    if (Str::Eq(name, KEYBOARD_LAYOUT_NAMES[i])) {
      SetKeyboardLayout((KeyboardLayout)i);
      return true;
    }
  }

  return false;
}

void Key::SetKeyboardLayout(KeyboardLayout layout) {
  layoutTable = KEYBOARD_LAYOUT_TABLES[(int)layout];
}

void Key::Press(uint8_t key) { PressRaw(TranslateKey(key)); }

void Key::Release(uint8_t key) { ReleaseRaw(TranslateKey(key)); }

__attribute__((weak)) void Key::Flush() {}

uint8_t Key::TranslateKey(uint8_t key) {
  if (layoutTable == nullptr) {
    return key;
  }

  return layoutTable[key];
}

//---------------------------------------------------------------------------

__attribute__((weak)) bool Key::IsNumLockOn() { return isNumLockOn; }

__attribute__((weak)) void Key::SetIsNumLockOn(bool value) {
  isNumLockOn = value;
}

//---------------------------------------------------------------------------

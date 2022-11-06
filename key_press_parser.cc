//---------------------------------------------------------------------------

#include "key_press_parser.h"

#include "key_code.h"
#include "steno_key_code.h"
#include <string.h>

//---------------------------------------------------------------------------

struct KeyCodeName {
  const char *name;
  uint32_t keyCode;
};

// Alphabetically sorted for binary search to work.
// spellchecker: disable
constexpr KeyCodeName KEYS[] = {
    {"0", KeyCode::_0},
    {"1", KeyCode::_1},
    {"2", KeyCode::_2},
    {"3", KeyCode::_3},
    {"4", KeyCode::_4},
    {"5", KeyCode::_5},
    {"6", KeyCode::_6},
    {"7", KeyCode::_7},
    {"8", KeyCode::_8},
    {"9", KeyCode::_9},
    {"a", KeyCode::A},
    {"aacute", 0},
    {"acircumflex", 0},
    {"acute", 0},
    {"adiaeresis", 0},
    {"ae", 0},
    {"agrave", 0},
    {"alt", KeyCode::L_ALT},
    {"alt_l", KeyCode::L_ALT},
    {"alt_r", KeyCode::R_ALT},
    {"ampersand", KeyCode::_7},
    {"apostrophe", KeyCode::APOSTROPHE},
    {"aring", 0},
    {"asciicircum", 0},
    {"asciitilde", 0},
    {"asterisk", KeyCode::_8},
    {"at", KeyCode::_2},
    {"atilde", 0},
    {"b", KeyCode::B},
    {"backslash", KeyCode::BACKSLASH},
    {"backspace", KeyCode::BACKSPACE},
    {"bar", KeyCode::BACKSLASH},
    {"braceleft", KeyCode::L_BRACKET},
    {"braceright", KeyCode::R_BRACKET},
    {"bracketleft", KeyCode::L_BRACKET},
    {"bracketright", KeyCode::R_BRACKET},
    {"brokenbar", 0},
    {"c", KeyCode::C},
    {"ccedilla", 0},
    {"cedilla", 0},
    {"cent", 0},
    {"clear", 0},
    {"colon", KeyCode::SEMICOLON},
    {"comma", KeyCode::COMMA},
    {"command", KeyCode::L_META},
    {"command_l", KeyCode::L_META},
    {"command_r", KeyCode::R_META},
    {"control", KeyCode::L_CTRL},
    {"control_l", KeyCode::L_CTRL},
    {"control_r", KeyCode::R_CTRL},
    {"copyright", 0},
    {"currency", 0},
    {"d", KeyCode::D},
    {"degree", 0},
    {"delete", KeyCode::DELETE},
    {"diaeresis", 0},
    {"division", 0},
    {"dollar", KeyCode::_4},
    {"down", KeyCode::DOWN},
    {"e", KeyCode::E},
    {"eacute", 0},
    {"ecircumflex", 0},
    {"ediaeresis", 0},
    {"egrave", 0},
    {"equal", KeyCode::EQUAL},
    {"escape", KeyCode::ESC},
    {"eth", 0},
    {"exclam", KeyCode::_1},
    {"exclamdown", 0},
    {"f", KeyCode::F},
    {"f1", KeyCode::F1},
    {"f10", KeyCode::F10},
    {"f11", KeyCode::F11},
    {"f12", KeyCode::F12},
    {"f13", KeyCode::F13},
    {"f14", KeyCode::F14},
    {"f15", KeyCode::F15},
    {"f16", KeyCode::F16},
    {"f17", KeyCode::F17},
    {"f18", KeyCode::F18},
    {"f19", KeyCode::F19},
    {"f2", KeyCode::F2},
    {"f20", KeyCode::F20},
    {"f21", KeyCode::F21},
    {"f22", KeyCode::F22},
    {"f23", KeyCode::F23},
    {"f24", KeyCode::F24},
    {"f3", KeyCode::F3},
    {"f4", KeyCode::F4},
    {"f5", KeyCode::F5},
    {"f6", KeyCode::F6},
    {"f7", KeyCode::F7},
    {"f8", KeyCode::F8},
    {"f9", KeyCode::F9},
    {"g", KeyCode::G},
    {"grave", KeyCode::GRAVE},
    {"greater", KeyCode::DOT},
    {"guillemotleft", 0},
    {"guillemotright", 0},
    {"h", KeyCode::H},
    {"home", KeyCode::HOME},
    {"hyper_l", 0},
    {"hyper_r", 0},
    {"hyphen", KeyCode::MINUS},
    {"i", KeyCode::I},
    {"iacute", 0},
    {"icircumflex", 0},
    {"idiaeresis", 0},
    {"igrave", 0},
    {"insert", KeyCode::INSERT},
    {"j", KeyCode::J},
    {"k", KeyCode::K},
    {"kp_0", KeyCode::KP_0},
    {"kp_1", KeyCode::KP_1},
    {"kp_2", KeyCode::KP_2},
    {"kp_3", KeyCode::KP_3},
    {"kp_4", KeyCode::KP_4},
    {"kp_5", KeyCode::KP_5},
    {"kp_6", KeyCode::KP_6},
    {"kp_7", KeyCode::KP_7},
    {"kp_8", KeyCode::KP_8},
    {"kp_9", KeyCode::KP_9},
    {"kp_add", KeyCode::KP_PLUS},
    {"kp_comma", KeyCode::KP_COMMA},
    {"kp_down", KeyCode::KP_2},
    {"kp_enter", KeyCode::KP_ENTER},
    {"kp_equal", KeyCode::KP_EQUAL},
    {"kp_left", KeyCode::KP_4},
    {"kp_minus", KeyCode::KP_MINUS},
    {"kp_multiply", KeyCode::KP_ASTERISK},
    {"kp_plus", KeyCode::KP_PLUS},
    {"kp_right", KeyCode::KP_6},
    {"kp_slash", KeyCode::KP_SLASH},
    {"kp_up", KeyCode::KP_8},
    {"l", KeyCode::L},
    {"left", KeyCode::LEFT},
    {"less", KeyCode::COMMA},
    {"m", KeyCode::M},
    {"macron", 0},
    {"masculine", 0},
    {"meta_l", KeyCode::L_ALT},
    {"meta_r", KeyCode::R_ALT},
    {"minus", KeyCode::MINUS},
    {"mu", 0},
    {"multiply", 0},
    {"n", KeyCode::N},
    {"nobreakspace", 0},
    {"notsign", 0},
    {"ntilde", 0},
    {"numbersign", 0},
    {"o", KeyCode::O},
    {"oacute", 0},
    {"ocircumflex", 0},
    {"odiaeresis", 0},
    {"ograve", 0},
    {"onehalf", 0},
    {"onequarter", 0},
    {"onesuperior", 0},
    {"ooblique", 0},
    {"ordfeminine", 0},
    {"oslash", 0},
    {"otilde", 0},
    {"p", KeyCode::P},
    {"page_down", KeyCode::PAGE_DOWN},
    {"page_up", KeyCode::PAGE_UP},
    {"paragraph", 0},
    {"parenleft", KeyCode::_9},
    {"parenright", KeyCode::_0},
    {"percent", KeyCode::_5},
    {"period", KeyCode::DOT},
    {"periodcentered", 0},
    {"plus", KeyCode::EQUAL},
    {"plusminus", 0},
    {"q", KeyCode::Q},
    {"question", KeyCode::SLASH},
    {"questiondown", 0},
    {"quotedbl", KeyCode::APOSTROPHE},
    {"quoteleft", 0},
    {"quoteright", 0},
    {"r", KeyCode::R},
    {"registered", 0},
    {"return", KeyCode::ENTER},
    {"right", KeyCode::RIGHT},
    {"s", KeyCode::S},
    {"scroll_lock", KeyCode::SCROLL_LOCK},
    {"section", 0},
    {"semicolon", KeyCode::SEMICOLON},
    {"shift", KeyCode::L_SHIFT},
    {"shift_l", KeyCode::L_SHIFT},
    {"shift_r", KeyCode::R_SHIFT},
    {"slash", KeyCode::SLASH},
    {"space", KeyCode::SPACE},
    {"ssharp", 0},
    {"sterling", 0},
    {"super", KeyCode::L_META},
    {"super_l", KeyCode::L_META},
    {"super_r", KeyCode::R_META},
    {"t", KeyCode::T},
    {"tab", KeyCode::TAB},
    {"thorn", 0},
    {"threequarters", 0},
    {"threesuperior", 0},
    {"twosuperior", 0},
    {"u", KeyCode::U},
    {"uacute", 0},
    {"ucircumflex", 0},
    {"udiaeresis", 0},
    {"ugrave", 0},
    {"underscore", KeyCode::MINUS},
    {"up", KeyCode::UP},
    {"v", KeyCode::V},
    {"w", KeyCode::W},
    {"windows", KeyCode::L_META},
    {"windows_l", KeyCode::L_META},
    {"windows_r", KeyCode::R_META},
    {"x", KeyCode::X},
    {"y", KeyCode::Y},
    {"yacute", 0},
    {"ydiaeresis", 0},
    {"yen", 0},
    {"z", KeyCode::Z},
};
// spellchecker: enable

static const KeyCodeName *GetKeyCodeName(const char *name) {
  size_t left = 0;
  size_t right = sizeof(KEYS) / sizeof(*KEYS);

  while (left < right) {
    size_t mid = (left + right) / 2;
    int comparison = strcmp(name, KEYS[mid].name);
    if (comparison < 0) {
      right = mid;
    } else if (comparison > 0) {
      left = mid + 1;
    } else {
      return &KEYS[mid];
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------

void StenoKeyPressTokenizer::ProcessNextToken() {
  while (p < end && IsWhitespace(*p)) {
    ++p;
  }

  if (p == end) {
    nextToken.type = StenoKeyPressToken::Type::End;
    return;
  }

  if (*p == '(') {
    ++p;
    nextToken.type = StenoKeyPressToken::Type::OpenParen;
    return;
  }

  if (*p == ')') {
    ++p;
    nextToken.type = StenoKeyPressToken::Type::CloseParen;
    return;
  }

  if (!IsWordCharacter(*p)) {
    ++p;
    nextToken.type = StenoKeyPressToken::Type::Unknown;
    return;
  }

  char buffer[32];
  char *write = buffer;
  while (p < end && IsWordCharacter(*p)) {
    if (write < buffer + 31) {
      *write++ = ToLower(*p);
    }
    ++p;
  }
  *write++ = '\0';

  const KeyCodeName *entry = GetKeyCodeName(buffer);
  if (!entry) {
    nextToken.type = StenoKeyPressToken::Type::Unknown;
    return;
  }
  nextToken.type = StenoKeyPressToken::Type::Key;
  nextToken.keyCode = entry->keyCode;
}

//---------------------------------------------------------------------------

#include "unit_test.h"
#include <assert.h>

TEST_BEGIN("KeyPressParser tests") {
  assert(GetKeyCodeName("escape")->keyCode == KeyCode::ESC);
  assert(GetKeyCodeName("page_up")->keyCode == KeyCode::PAGE_UP);
  assert(GetKeyCodeName("space")->keyCode == KeyCode::SPACE);

  const char *test = "Shift_L(h a p) p y";
  StenoKeyPressTokenizer tokenizer(test, test + strlen(test));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::Key, KeyCode::L_SHIFT));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::OpenParen));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::Key, KeyCode::H));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::Key, KeyCode::A));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::Key, KeyCode::P));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::CloseParen));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::Key, KeyCode::P));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::Key, KeyCode::Y));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::End));
}
TEST_END

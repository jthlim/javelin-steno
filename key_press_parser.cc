//---------------------------------------------------------------------------

#include "key_press_parser.h"

#include "key_code.h"
#include "unicode.h"
#include <string.h>

//---------------------------------------------------------------------------

struct KeyCodeName {
  const char *name;
  KeyCode keyCode;
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
    {"again", KeyCode::AGAIN},
    {"alt", KeyCode::L_ALT},
    {"alt_l", KeyCode::L_ALT},
    {"alt_r", KeyCode::R_ALT},
    {"ampersand", KeyCode::_7},
    {"apostrophe", KeyCode::APOSTROPHE},
    {"asterisk", KeyCode::_8},
    {"at", KeyCode::_2},
    {"audiolowervolume", KeyCode::CONSUMER_PAGE_VOLUME_DOWN},
    {"audiomute", KeyCode::CONSUMER_PAGE_MUTE},
    {"audionext", KeyCode::CONSUMER_PAGE_SCAN_NEXT_TRACK},
    {"audioplay", KeyCode::CONSUMER_PAGE_PLAY_PAUSE},
    {"audioprev", KeyCode::CONSUMER_PAGE_SCAN_PREVIOUS_TRACK},
    {"audioraisevolume", KeyCode::CONSUMER_PAGE_VOLUME_UP},
    {"audiostop", KeyCode::CONSUMER_PAGE_STOP},
    {"b", KeyCode::B},
    {"backslash", KeyCode::BACKSLASH},
    {"backslash_pipe", KeyCode::BACKSLASH_PIPE},
    {"backspace", KeyCode::BACKSPACE},
    {"bar", KeyCode::BACKSLASH},
    {"braceleft", KeyCode::L_BRACKET},
    {"braceright", KeyCode::R_BRACKET},
    {"bracketleft", KeyCode::L_BRACKET},
    {"bracketright", KeyCode::R_BRACKET},
    {"c", KeyCode::C},
    {"caps", KeyCode::CAPS_LOCK},
    {"caps_lock", KeyCode::CAPS_LOCK},
    {"clear", KeyCode::CLEAR},
    {"colon", KeyCode::SEMICOLON},
    {"comma", KeyCode::COMMA},
    {"command", KeyCode::L_META},
    {"command_l", KeyCode::L_META},
    {"command_r", KeyCode::R_META},
    {"control", KeyCode::L_CTRL},
    {"control_l", KeyCode::L_CTRL},
    {"control_r", KeyCode::R_CTRL},
    {"copy", KeyCode::COPY},
    {"cut", KeyCode::CUT},
    {"d", KeyCode::D},
    {"delete", KeyCode::DELETE},
    {"dollar", KeyCode::_4},
    {"down", KeyCode::DOWN},
    {"e", KeyCode::E},
    {"eject", KeyCode::CONSUMER_PAGE_EJECT},
    {"end", KeyCode::END},
    {"equal", KeyCode::EQUAL},
    {"escape", KeyCode::ESC},
    {"exclam", KeyCode::_1},
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
    {"find", KeyCode::FIND},
    {"g", KeyCode::G},
    {"grave", KeyCode::GRAVE},
    {"greater", KeyCode::DOT},
    {"h", KeyCode::H},
    {"hangeul", KeyCode::HANGEUL},
    {"hanja", KeyCode::HANJA},
    {"hankaku_zenkaku", KeyCode::HANKAKU_ZENKAKU},
    {"help", KeyCode::HELP},
    {"henkan", KeyCode::HENKAN},
    {"hiragana", KeyCode::HIRAGANA},
    {"home", KeyCode::HOME},
    {"hyphen", KeyCode::MINUS},
    {"i", KeyCode::I},
    {"insert", KeyCode::INSERT},
    {"j", KeyCode::J},
    {"k", KeyCode::K},
    {"kana", KeyCode::KANA},
    {"katakana", KeyCode::KATAKANA},
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
    {"kp_asterisk", KeyCode::KP_ASTERISK},
    {"kp_comma", KeyCode::KP_COMMA},
    {"kp_dot", KeyCode::KP_DOT},
    {"kp_down", KeyCode::KP_2},
    {"kp_enter", KeyCode::KP_ENTER},
    {"kp_equal", KeyCode::KP_EQUAL},
    {"kp_jp_comma", KeyCode::KP_JP_COMMA},
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
    {"media_eject_cd", KeyCode::CONSUMER_PAGE_EJECT},
    {"media_mute", KeyCode::CONSUMER_PAGE_MUTE},
    {"media_next_song", KeyCode::CONSUMER_PAGE_SCAN_NEXT_TRACK},
    {"media_play_pause", KeyCode::CONSUMER_PAGE_PLAY_PAUSE},
    {"media_previous_song", KeyCode::CONSUMER_PAGE_SCAN_PREVIOUS_TRACK},
    {"media_stop", KeyCode::CONSUMER_PAGE_STOP},
    {"media_volume_down", KeyCode::CONSUMER_PAGE_VOLUME_DOWN},
    {"media_volume_up", KeyCode::CONSUMER_PAGE_VOLUME_UP},
    {"menu", KeyCode::MENU},
    {"meta", KeyCode::L_META},
    {"meta_l", KeyCode::L_META},
    {"meta_r", KeyCode::R_META},
    {"minus", KeyCode::MINUS},
    {"muhenkan", KeyCode::MUHENKAN},
    {"mute", KeyCode::MUTE},
    {"n", KeyCode::N},
    {"o", KeyCode::O},
    {"open", KeyCode::OPEN},
    {"p", KeyCode::P},
    {"page_down", KeyCode::PAGE_DOWN},
    {"page_up", KeyCode::PAGE_UP},
    {"parenleft", KeyCode::_9},
    {"parenright", KeyCode::_0},
    {"paste", KeyCode::PASTE},
    {"pause", KeyCode::PAUSE},
    {"percent", KeyCode::_5},
    {"period", KeyCode::DOT},
    {"plus", KeyCode::EQUAL},
    {"power", KeyCode::POWER},
    {"print_screen", KeyCode::PRINT_SCREEN},
    {"q", KeyCode::Q},
    {"question", KeyCode::SLASH},
    {"quotedbl", KeyCode::APOSTROPHE},
    {"r", KeyCode::R},
    {"return", KeyCode::ENTER},
    {"right", KeyCode::RIGHT},
    {"ro", KeyCode::RO},
    {"s", KeyCode::S},
    {"scroll_lock", KeyCode::SCROLL_LOCK},
    {"select", KeyCode::SELECT},
    {"semicolon", KeyCode::SEMICOLON},
    {"shift", KeyCode::L_SHIFT},
    {"shift_l", KeyCode::L_SHIFT},
    {"shift_r", KeyCode::R_SHIFT},
    {"slash", KeyCode::SLASH},
    {"space", KeyCode::SPACE},
    {"stop", KeyCode::STOP},
    {"super", KeyCode::L_META},
    {"super_l", KeyCode::L_META},
    {"super_r", KeyCode::R_META},
    {"t", KeyCode::T},
    {"tab", KeyCode::TAB},
    {"u", KeyCode::U},
    {"underscore", KeyCode::MINUS},
    {"undo", KeyCode::UNDO},
    {"up", KeyCode::UP},
    {"v", KeyCode::V},
    {"volume_down", KeyCode::VOLUME_DOWN},
    {"volume_up", KeyCode::VOLUME_UP},
    {"w", KeyCode::W},
    {"windows", KeyCode::L_META},
    {"windows_l", KeyCode::L_META},
    {"windows_r", KeyCode::R_META},
    {"x", KeyCode::X},
    {"y", KeyCode::Y},
    {"yen", KeyCode::YEN},
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
  while (p < end && Unicode::IsWhitespace(*p)) {
    ++p;
  }

  if (p == end) {
    nextToken.type = StenoKeyPressToken::Type::END;
    return;
  }

  if (*p == '(') {
    ++p;
    nextToken.type = StenoKeyPressToken::Type::OPEN_PAREN;
    return;
  }

  if (*p == ')') {
    ++p;
    nextToken.type = StenoKeyPressToken::Type::CLOSE_PAREN;
    return;
  }

  if (!Unicode::IsWordCharacter(*p)) {
    ++p;
    nextToken.type = StenoKeyPressToken::Type::UNKNOWN;
    return;
  }

  char buffer[32];
  char *write = buffer;
  while (p < end && Unicode::IsWordCharacter(*p)) {
    if (write < buffer + 31) {
      *write++ = Unicode::ToLower(*p);
    }
    ++p;
  }
  *write++ = '\0';

  const KeyCodeName *entry = GetKeyCodeName(buffer);
  if (!entry) {
    nextToken.type = StenoKeyPressToken::Type::UNKNOWN;
    return;
  }
  nextToken.type = StenoKeyPressToken::Type::KEY;
  nextToken.keyCode = entry->keyCode;
}

//---------------------------------------------------------------------------
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
         StenoKeyPressToken(StenoKeyPressToken::Type::KEY, KeyCode::L_SHIFT));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::OPEN_PAREN));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::KEY, KeyCode::H));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::KEY, KeyCode::A));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::KEY, KeyCode::P));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::CLOSE_PAREN));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::KEY, KeyCode::P));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::KEY, KeyCode::Y));
  assert(tokenizer.GetNext() ==
         StenoKeyPressToken(StenoKeyPressToken::Type::END));
}
TEST_END

TEST_BEGIN("KeyPressParser sort test") {
  for (size_t i = 0; i < sizeof(KEYS) / sizeof(*KEYS); ++i) {
    const char *keyName = KEYS[i].name;
    assert(GetKeyCodeName(keyName) != nullptr);
  }
}
TEST_END

//---------------------------------------------------------------------------

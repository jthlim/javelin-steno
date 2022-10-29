//---------------------------------------------------------------------------

#include "steno_key_code_buffer.h"

#include "engine.h"
#include "key_press_parser.h"
#include "segment.h"
#include "state.h"
#include "string_util.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

char StenoKeyCodeBuffer::orthographicScratchPad[256];

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::Reset() {
  count = 0;
  addTranslationCount = 0;
  state.caseMode = StenoCaseMode::NORMAL;
  state.joinNext = false;
  state.isGlue = false;
  state.spaceCharacterLength = 1;
  state.spaceCharacter = " ";
}

void StenoKeyCodeBuffer::Populate(StenoTokenizer *tokenizer,
                                  StenoEngine &engine) {
  Reset();
  Append(tokenizer, engine);
}

void StenoKeyCodeBuffer::Append(StenoTokenizer *tokenizer,
                                StenoEngine &engine) {
  while (tokenizer->HasMore()) {
    StenoToken token = tokenizer->GetNext();
    if (token.state != nullptr) {
      state = *token.state;
    }
    if (token.text[0] == '{') {
      ProcessCommand(token.text, engine);
    } else {
      ProcessText(token.text);
    }
  }
}

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::ProcessText(const char *text) {
  bool isAutoGlue = IsGlue(text);
  if (!state.joinNext && !(isAutoGlue && state.isGlue)) {
    AppendText(state.spaceCharacter, state.spaceCharacterLength,
               StenoCaseMode::NORMAL);
  }

  AppendText(text, strlen(text), state.caseMode);
  state.joinNext = false;
  state.isGlue = isAutoGlue;
  state.caseMode = state.GetNextWordCaseMode();
}

void StenoKeyCodeBuffer::AppendText(const char *p, size_t n,
                                    StenoCaseMode caseMode) {
  const char *end = p + n;
  Utf8Pointer utf8p(p);

  while (utf8p < end) {
    uint32_t c = *utf8p;
    ++utf8p;

    if (c == '\\' && utf8p < end) {
      c = *utf8p;
      ++utf8p;

      switch (c) {
      case '{':
      case '}':
      case '\\':
      case '\"':
      case '\'':
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      default:
        buffer[count++] = StenoKeyCode('\\', StenoCaseMode::NORMAL);
        caseMode = GetNextLetterCaseMode(caseMode);
      }
    }

    buffer[count++] = StenoKeyCode(c, caseMode);
    caseMode = GetNextLetterCaseMode(caseMode);
  }
}

bool StenoKeyCodeBuffer::IsGlue(const char *p) {
  // Pure digits are considered glue.
  while (*p) {
    if (!IsAsciiDigit(*p)) {
      return false;
    }
    ++p;
  }
  return true;
}

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::ProcessCommand(const char *p, StenoEngine &engine) {
  const char *end = p + strlen(p);

  assert(*p == '{');
  assert(end[-1] == '}');
  --end;

  if (p[1] == '}') {
    // Cancel formatting
    state.isGlue = false;
    state.joinNext = false;
    state.caseMode = StenoCaseMode::NORMAL;
    return;
  }

  if (p[1] == '^') {
    p += 2;
    // Either orthographic suffix, non-orthographic suffix, or infix.

    StenoCaseMode caseMode = StenoCaseMode::UNSPECIFIED;
    if (p[0] == '~' && p[1] == '|') {
      p += 2;
      caseMode = state.caseMode;
    }

    if (*p == '}') {
      // suffix
      state.joinNext = true;
      return;
    }
    if (end[-1] == '^') {
      // Infix.
      AppendText(p, end - 1 - p, state.caseMode);
      state.caseMode = caseMode != StenoCaseMode::UNSPECIFIED
                           ? caseMode
                           : state.GetNextWordCaseMode();

      state.joinNext = true;
      return;
    }

    // Orthographic prefix.
    ProcessOrthographicSuffix(p, end - p, engine);
    if (caseMode != StenoCaseMode::UNSPECIFIED) {
      state.caseMode = caseMode;
    }
    return;
  }

  // Prefix
  if (end[-1] == '^') {
    ++p;
    StenoCaseMode caseMode = StenoCaseMode::UNSPECIFIED;
    if (p[0] == '~' && p[1] == '|') {
      p += 2;
      caseMode = state.caseMode;
    }

    if (!state.joinNext) {
      AppendText(state.spaceCharacter, state.spaceCharacterLength,
                 StenoCaseMode::NORMAL);
    }
    AppendText(p, end - 1 - p, state.caseMode);
    state.caseMode = caseMode != StenoCaseMode::UNSPECIFIED
                         ? caseMode
                         : state.GetNextWordCaseMode();
    state.joinNext = true;
    return;
  }

  if (p[1] == '*') {
    // Retroactive uppercase
    if (p[2] == '<' && p + 3 == end) {
      RetroactiveUpperCase(1);
      return;
    }

    // Retroactive capitalize
    if (p[2] == '-' && p[3] == '|' && p + 4 == end) {
      RetroactiveCapitalize(1);
      return;
    }

    // Retroactive delete space
    if (p[2] == '!' && p + 3 == end) {
      RetroactiveDeleteSpace();
      return;
    }
  }

  if (p + 2 == end) {
    switch (p[1]) {
    case '.':
    case '?':
    case '!':
      AppendText(p + 1, 1, StenoCaseMode::NORMAL);
      state.joinNext = false;
      state.caseMode = StenoCaseMode::TITLE_ONCE;
      return;

    case ',':
    case ':':
    case ';':
      AppendText(p + 1, 1, StenoCaseMode::NORMAL);
      state.joinNext = false;
      state.caseMode = StenoCaseMode::NORMAL;
      return;
    }
  }

  // Carry casing.
  if (p[1] == '~' && p[2] == '|') {
    p += 2;
    AppendText(p, end - p, state.caseMode);
    return;
  }

  // Capitalize next word.
  if (p[1] == '-' && p[2] == '|') {
    p += 3;
    AppendText(p, end - p, state.caseMode);
    state.caseMode = StenoCaseMode::TITLE_ONCE;
    return;
  }

  if (p[1] == '<' && p + 2 == end) {
    // Upper case next word.
    state.caseMode = StenoCaseMode::UPPER_ONCE;
    return;
  }

  if (p[1] == '>' && p + 2 == end) {
    // Lower case next word.
    state.caseMode = StenoCaseMode::LOWER_ONCE;
    return;
  }

  if (p[1] == '&') {
    p += 2;
    // Glue!
    if (!state.joinNext && !state.isGlue) {
      AppendText(state.spaceCharacter, state.spaceCharacterLength,
                 StenoCaseMode::NORMAL);
    }
    AppendText(p, end - p, state.caseMode);
    state.caseMode = state.GetNextWordCaseMode();
    state.joinNext = false;
    state.isGlue = true;
    return;
  }

  if (p[1] == ':') {
    List<char *> parameters;
    p += 2;
    for (;;) {
      char *tokenEnd = (char *)memchr(p, ':', end - p);
      if (tokenEnd == nullptr) {
        parameters.Add(strndup(p, end - p));
        break;
      } else {
        parameters.Add(strndup(p, tokenEnd - p));
        p = tokenEnd + 1;
      }
    }

    bool handled = ProcessFunction(parameters);
    for (size_t i = 0; i < parameters.GetCount(); ++i) {
      free(parameters[i]);
    }
    if (handled)
      return;
  }

  if (p[1] == '#') {
    if (ProcessKeyPresses(p + 2, end)) {
      return;
    }
  }

  static const char ADD_TRANSLATION[] = "JAVELIN:ADD_TRANSLATION";
  if (p + sizeof(ADD_TRANSLATION) == end &&
      memcmp(p + 1, ADD_TRANSLATION, sizeof(ADD_TRANSLATION) - 1) == 0) {
    ++addTranslationCount;
    return;
  }

  /// Just display the unhandled command.
  AppendText(p, end + 1 - p, StenoCaseMode::NORMAL);
}

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::ProcessOrthographicSuffix(const char *text,
                                                   size_t length,
                                                   StenoEngine &engine) {
  char *suffix = strndup(text, length);

  size_t start = count;
  while (start != 0 && count - start < sizeof(orthographicScratchPad) / 4 - 1 &&
         buffer[start - 1].IsLetter()) {
    --start;
  }

  Utf8Pointer utf8p(orthographicScratchPad);
  for (size_t i = start; i < count; ++i) {
    utf8p.SetAndAdvance(buffer[i].GetUnicode());
  }
  utf8p.Set(0);

  char *word = engine.AddSuffix(orthographicScratchPad, suffix);

  count = start;

  char *pWord = word;
  char *pScratchPad = orthographicScratchPad;

  // Skip the unchanged prefixes -- this preserves camelCasing when suffixes are
  // added.
  while (*pWord && *pScratchPad && *pWord == *pScratchPad) {
    // Skip utf8 suffix characters.
    if ((*pWord & 0xc0) != 0x80) {
      state.caseMode = buffer[count++].GetCaseMode();
    }
    ++pWord;
    ++pScratchPad;
  }

  AppendText(pWord, strlen(pWord), GetNextLetterCaseMode(state.caseMode));
  state.caseMode = state.GetNextWordCaseMode();

  free(word);
  free(suffix);
}

//---------------------------------------------------------------------------

// Only for debug.
char *StenoKeyCodeBuffer::ToString() {
  size_t length = 1;
  for (size_t i = 0; i < count; ++i) {
    if (!buffer[i].IsRawKeyCode()) {
      length += Utf8Pointer::BytesForCharacterCode(buffer[i].ResolveUnicode());
    }
  }
  char *result = (char *)malloc(length);
  Utf8Pointer utf8p(result);
  for (size_t i = 0; i < count; ++i) {
    if (!buffer[i].IsRawKeyCode()) {
      utf8p.SetAndAdvance(buffer[i].ResolveUnicode());
    }
  }
  utf8p.Set(0);

  return result;
}

//---------------------------------------------------------------------------

// Helper method to do rotates in place.
//
// spellchecker: disable
//
// Given: `ABCDE`
// Reverse A-C, and D-E: `CBAED`
// Reverse the whole lot: `DEABC`
//
// spellchecker: enable
void StenoKeyCodeBuffer::Reverse(StenoKeyCode *start, StenoKeyCode *end) {
  --end;
  while (start < end) {
    StenoKeyCode t = *start;
    *start = *end;
    *end = t;
    ++start;
    --end;
  }
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::ProcessKeyPresses(const char *p, const char *end) {
  StenoKeyPressTokenizer tokenizer(p, end);
  List<uint32_t> keyPressStack;

  for (;;) {
    StenoKeyPressToken token = tokenizer.GetNext();
    switch (token.type) {
    case StenoKeyPressToken::Type::Key: {
      uint32_t keyCode = token.keyCode;
      if (keyCode != 0) {
        buffer[count++] = StenoKeyCode::CreateRawKeyCodePress(keyCode);
      }
      if (tokenizer.PeekNextTokenType() ==
          StenoKeyPressToken::Type::OpenParen) {
        keyPressStack.Add(keyCode);
        tokenizer.GetNext();
      } else {
        buffer[count++] = StenoKeyCode::CreateRawKeyCodeRelease(keyCode);
      }
      break;
    }
    case StenoKeyPressToken::Type::Unknown:
      return false;
    case StenoKeyPressToken::Type::OpenParen:
      // Unexpected open paren.
      return false;
    case StenoKeyPressToken::Type::CloseParen: {
      if (keyPressStack.IsEmpty()) {
        return false;
      }
      uint32_t keyCode = keyPressStack.Back();
      if (keyCode != 0) {
        buffer[count++] = StenoKeyCode::CreateRawKeyCodeRelease(keyCode);
      }
      keyPressStack.Pop();
      break;
    }
    case StenoKeyPressToken::Type::End: {
      // If unmatched trailing brackets, just close them out..
      while (keyPressStack.IsNotEmpty()) {
        uint32_t keyCode = keyPressStack.Back();
        if (keyCode != 0) {
          buffer[count++] =
              StenoKeyCode::CreateRawKeyCodeRelease(keyPressStack.Back());
        }
        keyPressStack.Pop();
      }

      return true;
    }
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "key_code.h"
#include "unit_test.h"

TEST_BEGIN("StenoKeyCodeBuffer tests") {
  StenoKeyCodeBuffer *buffer = new StenoKeyCodeBuffer();

  const char *test = "Shift_L(h a p) p y";
  buffer->ProcessKeyPresses(test, test + strlen(test));

  // clang-format off
  assert(buffer->buffer[0] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::L_SHIFT));
  assert(buffer->buffer[1] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::H));
  assert(buffer->buffer[2] == StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::H));
  assert(buffer->buffer[3] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::A));
  assert(buffer->buffer[4] == StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::A));
  assert(buffer->buffer[5] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::P));
  assert(buffer->buffer[6] == StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::P));
  assert(buffer->buffer[7] == StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::L_SHIFT));
  assert(buffer->buffer[8] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::P));
  assert(buffer->buffer[9] == StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::P));
  assert(buffer->buffer[10] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::Y));
  assert(buffer->buffer[11] == StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::Y));
  // clang-format on

  delete buffer;
}
TEST_END

//---------------------------------------------------------------------------
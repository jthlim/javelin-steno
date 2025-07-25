//---------------------------------------------------------------------------

#include "steno_key_code_buffer.h"

#include "key_press_parser.h"
#include "orthography.h"
#include "segment.h"
#include "state.h"
#include "str.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::Reset() {
  wasLastActionAStitch = false;
  launchConsole = false;
  launchAddTranslation = false;
  doResetState = false;
  state.Reset();
  currentOutput = buffer;
  lastText = buffer;
}

void StenoKeyCodeBuffer::Populate(StenoTokenizer &tokenizer,
                                  bool executeSideEffects) {
  Reset();
  Append(tokenizer, executeSideEffects);
}

void StenoKeyCodeBuffer::Append(StenoTokenizer &tokenizer,
                                bool executeSideEffects) {
  for (const StenoToken token : tokenizer) {
    if (token.state != nullptr) {
      state = *token.state;
    }
    if (token.text[0] == '{') {
      this->executeSideEffects = token.isLastSegment && executeSideEffects;
      ProcessCommand(token.text, token.length);
    } else {
      lastText = currentOutput;
      ProcessText(token.text, token.length);
    }
  }
}

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::ProcessText(const char *text, size_t length) {
  const bool isAutoGlue = IsGlue(text);
  if (!state.joinNext && !(isAutoGlue && state.isGlue)) {
    AppendText(state.GetSpace(), state.spaceLength, StenoCaseMode::NORMAL);
  }

  AppendText(text, length, state.caseMode);
  state.joinNext = false;
  state.isGlue = isAutoGlue;
  state.isManualStateChange = false;
  state.caseMode = state.GetNextWordCaseMode();
}

// hasManualStateChange tags a letter as the appropriate case for the purpose
// of reverse lookups.
//
// This enables looking up: KPA*/RAOD -> RAO*D
// and AFL/PW*ET/{:retro_lower:1} -> A*FL/PW*ET
void StenoKeyCodeBuffer::AppendText(const char *p, size_t n,
                                    StenoCaseMode caseMode) {
  if (state.overrideCaseMode != StenoCaseMode::NORMAL) {
    caseMode = state.overrideCaseMode;
  }
  AppendTextNoCaseModeOverride(p, n, caseMode);
}

void StenoKeyCodeBuffer::AppendTextNoCaseModeOverride(const char *p, size_t n,
                                                      StenoCaseMode caseMode) {
  const char *end = p + n;
  Utf8Pointer utf8p(p);
  StenoKeyCode *d = currentOutput;

  while (utf8p < end) {
    uint32_t c = *utf8p++;

    if (c == '\\') [[unlikely]] {
      if (utf8p < end) [[likely]] {
        c = *utf8p++;
        switch (c) {
        case '{':
        case '}':
        case '\\':
        case '\"':
        case '\'':
        case ' ':
        case ':':
          break;
        case 'b':
          c = '\b';
          break;
        case 'f':
          c = '\f';
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
          *d++ = StenoKeyCode('\\', StenoCaseMode::NORMAL);
          caseMode = GetNextLetterCaseMode(caseMode);
        }
      }
    }

    if (c == '\b') [[unlikely]] {
      currentOutput = d;
      Backspace(1);
      d = currentOutput;
    } else {
      *d++ = StenoKeyCode(c, caseMode, StenoCaseMode::NORMAL);
    }

    caseMode = GetNextLetterCaseMode(caseMode);
  }

  currentOutput = d;
  wasLastActionAStitch = false;
}

bool StenoKeyCodeBuffer::IsGlue(const char *p) {
  // Pure digits are considered glue.
  while (*p) {
    if (!Unicode::IsAsciiDigit(*p)) {
      return false;
    }
    ++p;
  }
  return true;
}

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::ProcessCommand(const char *p, size_t length) {
  const char *end = p + length;

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

    if (p == end) {
      // {^} handling.
      state.caseMode = caseMode != StenoCaseMode::UNSPECIFIED
                           ? caseMode
                           : state.GetNextWordCaseMode();
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

    // Orthographic suffix.
    ProcessOrthographicSuffix(p, end - p);
    if (caseMode != StenoCaseMode::UNSPECIFIED) {
      state.caseMode = caseMode;
    }
    state.isGlue = false;
    state.joinNext = false;
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
      AppendText(state.GetSpace(), state.spaceLength, StenoCaseMode::NORMAL);
    }
    AppendText(p, end - 1 - p, state.caseMode);
    state.caseMode = caseMode != StenoCaseMode::UNSPECIFIED
                         ? caseMode
                         : state.GetNextWordCaseMode();
    state.isGlue = false;
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

    // Retroactive un-capitalize.
    if (p[2] == '>' && p + 3 == end) {
      RetroactiveUncapitalize(1);
      return;
    }

    // Retroactive delete space
    if (p[2] == '!' && p + 3 == end) {
      RetroactiveDeleteSpace();
      return;
    }

    if (p[2] == '(' && end[-1] == ')') {
      RetroactiveFormatCurrency(p + 3, end - 1);
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
      state.isGlue = false;
      state.caseMode = StenoCaseMode::TITLE_ONCE;
      return;

    case ',':
    case ':':
    case ';':
      AppendText(p + 1, 1, StenoCaseMode::NORMAL);
      state.joinNext = false;
      state.isGlue = false;
      state.caseMode = StenoCaseMode::NORMAL;
      return;
    }
  }

  // Carry casing.
  if (p[1] == '~' && p[2] == '|') {
    p += 3;
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

  // Upper case next word.
  if (p[1] == '<' && p + 2 == end) {
    state.caseMode = StenoCaseMode::UPPER_ONCE;
    return;
  }

  // Lower case next word.
  if (p[1] == '>' && p + 2 == end) {
    state.caseMode = StenoCaseMode::LOWER_ONCE;
    return;
  }

  if (p[1] == '&') {
    p += 2;
    // Glue!
    if (!state.joinNext && !state.isGlue) {
      AppendText(state.GetSpace(), state.spaceLength, StenoCaseMode::NORMAL);
    }
    AppendText(p, end - p, state.caseMode);
    state.caseMode = state.GetNextWordCaseMode();
    state.joinNext = false;
    state.isGlue = true;
    return;
  }

  if (p[1] == ':') {
    // This is a special case for the steno tools to display more meaningful
    // data.
    if (p[2] == '=') {
      return;
    }

    List<char *> parameters;
    const char *token = p + 2;
    while (token) {
      token = AddParameter(parameters, token, end);
    }

    const bool handled = ProcessFunction(parameters);
    for (char *parameter : parameters) {
      free(parameter);
    }
    if (handled) {
      return;
    }
  }

  if (p[1] == '#') {
    if (!executeSideEffects) {
      return;
    }
    if (ProcessKeyPresses(p + 2, end)) {
      return;
    }
  }

  /// Just display the unhandled command.
  AppendText(p, end + 1 - p, StenoCaseMode::NORMAL);
}

const char *StenoKeyCodeBuffer::AddParameter(List<char *> &parameters,
                                             const char *p, const char *end) {
  const char *start = p;
  const char *result = nullptr;

  while (p < end) {
    const char c = *p;
    if (c == ':') {
      result = p + 1;
      break;
    }
    ++p;
    if (c == '\\') {
      ++p;
    }
  }
  parameters.Add(Str::DupN(start, p - start));
  return result;
}

void StenoKeyCodeBuffer::AppendSpace() {
  // Append a space, then shift it to the first keycode in the buffer.
  StenoKeyCode *startSpace = currentOutput;
  AppendText(state.GetSpace(), state.spaceLength, StenoCaseMode::NORMAL);
  StenoKeyCode *start = startSpace;
  while (start > buffer && start[-1].IsRawKeyCode()) {
    --start;
  }
  if (start == startSpace) {
    return;
  }

  Reverse(start, startSpace);
  Reverse(startSpace, currentOutput);
  Reverse(start, currentOutput);
}

//---------------------------------------------------------------------------

void StenoKeyCodeBuffer::ProcessOrthographicSuffix(const char *text,
                                                   size_t length) {
  char orthographicScratchPad[32];
  char *suffix = Str::DupN(text, length);

  StenoKeyCode *start = currentOutput;
  size_t byteCount = 1; // Need one byte for terminating null.
  while (start != lastText && start[-1].IsLetter()) {
    const size_t utf8Length =
        Utf8Pointer::BytesForCharacterCode(start[-1].GetUnicode());
    if (byteCount + utf8Length > sizeof(orthographicScratchPad)) {
      break;
    }
    byteCount += utf8Length;
    --start;
  }

  Utf8Pointer utf8p(orthographicScratchPad);
  for (const StenoKeyCode *p = start; p < currentOutput; ++p) {
    utf8p.SetAndAdvance(p->GetUnicode());
  }
  utf8p.SetTerminatingNull();

  char *word = orthography->AddSuffix(orthographicScratchPad, suffix);

  currentOutput = start;

  char *pWord = word;
  char *pScratchPad = orthographicScratchPad;

  // Skip the unchanged prefixes -- this preserves camelCasing when suffixes are
  // added.
  StenoCaseMode caseMode = StenoCaseMode::NORMAL;
  while (*pWord && *pScratchPad && *pWord == *pScratchPad) {
    // Skip utf8 suffix characters.
    if ((*pWord & 0xc0) != 0x80) {
      caseMode = currentOutput->GetOutputCaseMode();
      ++currentOutput;
    }
    ++pWord;
    ++pScratchPad;
  }
  if (state.caseMode == StenoCaseMode::NORMAL) {
    state.caseMode = GetNextLetterCaseMode(caseMode);
  }

  AppendTextNoCaseModeOverride(pWord, Str::Length(pWord), state.caseMode);
  state.caseMode = state.GetNextWordCaseMode();

  free(word);
  free(suffix);
}

//---------------------------------------------------------------------------

char *StenoKeyCodeBuffer::ToString(size_t startingOffset) const {
  char *result = (char *)malloc(GetCount() * 4 + 1);
  Utf8Pointer utf8p(result);
  for (const StenoKeyCode *p = buffer + startingOffset; p < currentOutput;
       ++p) {
    if (!p->IsRawKeyCode()) {
      utf8p.SetAndAdvance(p->ResolveOutputUnicode());
    }
  }
  utf8p.SetTerminatingNull();

  return result;
}

char *StenoKeyCodeBuffer::ToUnresolvedString() const {
  char *result = (char *)malloc(GetCount() * 4 + 1);
  Utf8Pointer utf8p(result);
  for (const StenoKeyCode *p = buffer; p < currentOutput; ++p) {
    if (!p->IsRawKeyCode()) {
      utf8p.SetAndAdvance(p->ResolveSelectedUnicode());
    }
  }
  utf8p.SetTerminatingNull();

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
    const StenoKeyCode t = *start;
    *start = *end;
    *end = t;
    ++start;
    --end;
  }
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::ProcessKeyPresses(const char *p, const char *end) {
  StenoKeyPressTokenizer tokenizer(p, end);
  List<KeyCode> keyPressStack;

  for (;;) {
    const StenoKeyPressToken token = tokenizer.GetNext();
    switch (token.type) {
    case StenoKeyPressToken::Type::KEY: {
      const KeyCode keyCode = token.keyCode;
      if (keyCode != 0) {
        *currentOutput++ = StenoKeyCode::CreateRawKeyCodePress(keyCode);
      }
      if (tokenizer.PeekNextTokenType() ==
          StenoKeyPressToken::Type::OPEN_PAREN) {
        keyPressStack.Add(keyCode);
        tokenizer.GetNext();
      } else {
        *currentOutput++ = StenoKeyCode::CreateRawKeyCodeRelease(keyCode);
      }
    } break;

    case StenoKeyPressToken::Type::UNKNOWN:    // Unknown token
    case StenoKeyPressToken::Type::OPEN_PAREN: // Unexpected open paren
      ReleaseKeyStack(keyPressStack);
      return false;

    case StenoKeyPressToken::Type::CLOSE_PAREN: {
      if (keyPressStack.IsEmpty()) {
        return false;
      }
      const KeyCode keyCode = keyPressStack.Back();
      if (keyCode != 0) {
        *currentOutput++ = StenoKeyCode::CreateRawKeyCodeRelease(keyCode);
      }
      keyPressStack.Pop();
    } break;

    case StenoKeyPressToken::Type::END:
      // If unmatched trailing brackets, just close them out.
      ReleaseKeyStack(keyPressStack);
      return true;
    }
  }
}

void StenoKeyCodeBuffer::ReleaseKeyStack(List<KeyCode> &keyPressStack) {
  while (keyPressStack.IsNotEmpty()) {
    const KeyCode keyCode = keyPressStack.Back();
    if (keyCode != 0) {
      *currentOutput++ = StenoKeyCode::CreateRawKeyCodeRelease(keyCode);
    }
    keyPressStack.Pop();
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "key_code.h"
#include "unit_test.h"

TEST_BEGIN("StenoKeyCodeBuffer tests") {
  StenoKeyCodeBuffer *buffer = new StenoKeyCodeBuffer();
  buffer->Reset();

  const char *test = "Shift_L(h a p) p y";
  buffer->ProcessKeyPresses(test, test + Str::Length(test));

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

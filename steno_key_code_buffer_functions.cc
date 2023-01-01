//---------------------------------------------------------------------------

#include "engine.h"
#include "key_code.h"
#include "str.h"

//---------------------------------------------------------------------------

struct KeyCodeFunctionEntry {
  const char *name;
  bool (StenoKeyCodeBuffer::*handler)(const List<char *> &parameters);
};

constexpr KeyCodeFunctionEntry HANDLERS[] = {
    {"add_translation", &StenoKeyCodeBuffer::AddTranslationFunction},
    {"disable_dictionary", &StenoKeyCodeBuffer::DisableDictionaryFunction},
    {"enable_dictionary", &StenoKeyCodeBuffer::EnableDictionaryFunction},
    {"keyboard_layout", &StenoKeyCodeBuffer::KeyboardLayoutFunction},
    {"reset_state", &StenoKeyCodeBuffer::ResetStateFunction},
    {"retro_capitalise", &StenoKeyCodeBuffer::RetroCapitalizeFunction},
    {"retro_double_quotes", &StenoKeyCodeBuffer::RetroDoubleQuotesFunction},
    {"retro_lower", &StenoKeyCodeBuffer::RetroLowerCaseFunction},
    {"retro_single_quotes", &StenoKeyCodeBuffer::RetroSingleQuotesFunction},
    {"retro_title", &StenoKeyCodeBuffer::RetroTitleCaseFunction},
    {"retro_upper", &StenoKeyCodeBuffer::RetroUpperCaseFunction},
    {"set_case", &StenoKeyCodeBuffer::SetCaseFunction},
    {"set_space", &StenoKeyCodeBuffer::SetSpaceFunction},
    {"toggle_dictionary", &StenoKeyCodeBuffer::ToggleDictionaryFunction},
    {"unicode", &StenoKeyCodeBuffer::UnicodeFunction},
};

//---------------------------------------------------------------------------

static bool ReadIntegerParameter(int &result, const char *p,
                                 bool allowNegative = false) {
  int multiplier = 1;
  if (allowNegative) {
    if (*p == '-') {
      multiplier = -1;
      ++p;
    }
  }

  int value = 0;
  while (*p) {
    if (*p < '0' || *p > '9') {
      return false;
    }
    value = 10 * value + *p - '0';
    ++p;
  }

  result = value * multiplier;

  return true;
}

//---------------------------------------------------------------------------

// `a b {#key1} c {#key2}` with two backspaces should leave:
// `a {#key1} {#key2}
void StenoKeyCodeBuffer::Backspace(int backspaceCount) {
  StenoKeyCode *pEnd = buffer + count;
  StenoKeyCode *d = pEnd;
  int localCount = backspaceCount;
  while (d > buffer) {
    --d;
    if (d->IsRawKeyCode()) {
      continue;
    }
    if (--localCount == 0) {
      break;
    }
  }

  StenoKeyCode *s = d;
  while (s < pEnd) {
    if (s->IsRawKeyCode() || backspaceCount == 0) {
      *d++ = *s++;
    } else {
      --backspaceCount;
      ++s;
    }
  }
  count = size_t(d - buffer);
}

void StenoKeyCodeBuffer::RetroactiveCapitalize(int wordCount) {
  if (count == 0) {
    return;
  }

  StenoKeyCode *p = buffer + count - 1;
  StenoKeyCode *lastCharacterPointer = p;

  while (wordCount > 0) {
    for (;;) {
      if (p < buffer) {
        goto epilog;
      }
      if (!p->IsWhitespace()) {
        break;
      }
      --p;
    }

    for (;;) {
      if (p < buffer) {
        goto epilog;
      }
      if (p->IsWhitespace()) {
        break;
      }
      if (p->IsLetter()) {
        lastCharacterPointer = p;
      }
      p->SetCase(StenoCaseMode::LOWER);
      --p;
    }

    --wordCount;
  }

epilog:
  lastCharacterPointer->SetCase(StenoCaseMode::TITLE_ONCE);
}

void StenoKeyCodeBuffer::RetroactiveUncapitalize(int wordCount) {
  StenoKeyCode *p = buffer + count - 1;
  while (wordCount > 0) {
    for (;;) {
      if (p < buffer) {
        return;
      }
      if (!p->IsWhitespace()) {
        break;
      }
      --p;
    }

    StenoKeyCode *lastCharacterPointer = p;
    for (;;) {
      if (p < buffer) {
        break;
      }
      if (p->IsWhitespace()) {
        break;
      }
      if (p->IsLetter()) {
        lastCharacterPointer = p;
      }
      --p;
    }
    lastCharacterPointer->SetCase(StenoCaseMode::LOWER_ONCE);

    --wordCount;
  }
}

void StenoKeyCodeBuffer::RetroactiveTitleCase(int wordCount) {
  StenoKeyCode *p = buffer + count - 1;
  while (wordCount > 0) {
    for (;;) {
      if (p < buffer) {
        return;
      }
      if (!p->IsWhitespace()) {
        break;
      }
      --p;
    }

    StenoKeyCode *lastCharacterPointer = p;
    for (;;) {
      if (p < buffer) {
        break;
      }
      if (p->IsWhitespace()) {
        break;
      }
      if (p->IsLetter()) {
        lastCharacterPointer = p;
      }
      p->SetCase(StenoCaseMode::LOWER);
      --p;
    }
    lastCharacterPointer->SetCase(StenoCaseMode::TITLE);

    --wordCount;
  }
}

void StenoKeyCodeBuffer::RetroactiveUpperCase(int wordCount) {
  StenoKeyCode *p = buffer + count - 1;
  while (wordCount > 0) {
    for (;;) {
      if (p < buffer) {
        return;
      }
      if (!p->IsWhitespace()) {
        break;
      }
      --p;
    }

    for (;;) {
      if (p < buffer) {
        return;
      }

      if (p->IsWhitespace()) {
        break;
      }
      p->SetCase(StenoCaseMode::UPPER);
      --p;
    }

    --wordCount;
  }
}

void StenoKeyCodeBuffer::RetroactiveLowerCase(int wordCount) {
  StenoKeyCode *p = buffer + count - 1;
  while (wordCount > 0) {
    for (;;) {
      if (p < buffer) {
        return;
      }
      if (!p->IsWhitespace()) {
        break;
      }
      --p;
    }

    for (;;) {
      if (p < buffer) {
        return;
      }

      if (p->IsWhitespace()) {
        break;
      }
      p->SetCase(StenoCaseMode::LOWER);
      --p;
    }

    --wordCount;
  }
}

void StenoKeyCodeBuffer::RetroactiveQuotes(int wordCount,
                                           const char *startQuote,
                                           const char *endQuote) {
  if (count == 0) {
    return;
  }

  StenoKeyCode *endQuoteBufferPointer = buffer + count;
  AppendText(endQuote, strlen(endQuote), StenoCaseMode::NORMAL);
  StenoKeyCode *startQuoteBufferPointer = buffer + count;
  AppendText(startQuote, strlen(startQuote), StenoCaseMode::NORMAL);

  StenoKeyCode *p = endQuoteBufferPointer - 1;
  while (wordCount) {
    for (;;) {
      if (p <= buffer) {
        break;
      }
      if (!p->IsWhitespace()) {
        break;
      }
      --p;
    }

    for (;;) {
      if (p <= buffer) {
        break;
      }

      if (p->IsWhitespace()) {
        break;
      }
      --p;
    }

    --wordCount;
  }
  if (p->IsWhitespace()) {
    ++p;
  }

  // Rotate in place.
  Reverse(p, startQuoteBufferPointer);
  Reverse(startQuoteBufferPointer, buffer + count);
  Reverse(p, buffer + count);
}

void StenoKeyCodeBuffer::RetroactiveDeleteSpace() {
  // Finds the first whitespace in the buffer and removes it.
  StenoKeyCode *end = buffer + count;
  StenoKeyCode *p = end - 1;
  while (p >= buffer) {
    if (p->IsWhitespace()) {
      count--;
      memmove(p, p + 1, sizeof(StenoKeyCode) * (end - p - 1));
      return;
    }
    --p;
  }
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::ProcessFunction(const List<char *> &parameters) {
  for (const KeyCodeFunctionEntry &entry : HANDLERS) {
    if (Str::Eq(entry.name, parameters[0])) {
      return (this->*entry.handler)(parameters);
    }
  }
  return false;
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::AddTranslationFunction(const List<char *> &) {
  addTranslationCount++;
  return true;
}

bool StenoKeyCodeBuffer::EnableDictionaryFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  ++resetStateCount;
  return rootDictionary->EnableDictionary(parameters[1]);
}

bool StenoKeyCodeBuffer::DisableDictionaryFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  ++resetStateCount;
  return rootDictionary->DisableDictionary(parameters[1]);
}

bool StenoKeyCodeBuffer::ToggleDictionaryFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  ++resetStateCount;
  return rootDictionary->ToggleDictionary(parameters[1]);
}

bool StenoKeyCodeBuffer::RetroCapitalizeFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveCapitalize(wordCount);
  return true;
}

bool StenoKeyCodeBuffer::RetroTitleCaseFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveTitleCase(wordCount);
  return true;
}

bool StenoKeyCodeBuffer::RetroUpperCaseFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveUpperCase(wordCount);
  return true;
}

bool StenoKeyCodeBuffer::RetroLowerCaseFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveLowerCase(wordCount);
  return true;
}

bool StenoKeyCodeBuffer::RetroSingleQuotesFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveQuotes(wordCount, "'", "'");
  return true;
}

bool StenoKeyCodeBuffer::RetroDoubleQuotesFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveQuotes(wordCount, "\"", "\"");
  return true;
}

bool StenoKeyCodeBuffer::SetCaseFunction(const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  if (Str::Eq(parameters[1], "normal")) {
    // This is needed because orthographic suffixes can make the case mode
    // take on the overriden case mode. This is not a problem for the other
    // cases as the override will kick in.
    state.caseMode = StenoCaseMode::NORMAL;
    state.overrideCaseMode = StenoCaseMode::NORMAL;
    return true;
  }

  if (Str::Eq(parameters[1], "lower")) {
    state.overrideCaseMode = StenoCaseMode::LOWER;
    return true;
  }

  if (Str::Eq(parameters[1], "upper")) {
    state.overrideCaseMode = StenoCaseMode::UPPER;
    return true;
  }

  if (Str::Eq(parameters[1], "title")) {
    state.overrideCaseMode = StenoCaseMode::TITLE;
    return true;
  }

  return false;
}

bool StenoKeyCodeBuffer::SetSpaceFunction(const List<char *> &parameters) {
  static int spaceOffset = 0;
  static char spaceBuffer[16];

  if (parameters.GetCount() != 2) {
    return false;
  }

  if (Str::Eq(parameters[1], " ")) {
    state.spaceCharacter = " ";
    state.spaceCharacterLength = 1;
    return true;
  }

  // Find if it exists in the buffer.
  size_t length = strlen(parameters[1]);
  for (size_t i = 0; i + length <= spaceOffset; ++i) {
    if (memcmp(spaceBuffer + i, parameters[1], length) == 0) {
      state.spaceCharacter = spaceBuffer + i;
      state.spaceCharacterLength = length;
      return true;
    }
  }

  // Can it fit?
  if (spaceOffset + length < sizeof(spaceBuffer)) {
    state.spaceCharacter = spaceBuffer + spaceOffset;
    state.spaceCharacterLength = length;
    memcpy(spaceBuffer + spaceOffset, parameters[1], length);
    spaceOffset += length;
    return true;
  }

  return false;
}

bool StenoKeyCodeBuffer::ResetStateFunction(const List<char *> &) {
  resetStateCount++;
  return true;
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::UnicodeFunction(const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  return StenoKeyCodeEmitter::SetUnicodeMode(parameters[1]);
}

bool StenoKeyCodeBuffer::KeyboardLayoutFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  return Key::SetKeyboardLayout(parameters[1]);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("StenoKeyCodeBuffer: Backspace() should give expected results") {
  StenoKeyCodeBuffer buffer;
  buffer.buffer[0] = StenoKeyCode('a', StenoCaseMode::NORMAL);
  buffer.buffer[1] = StenoKeyCode('b', StenoCaseMode::NORMAL);
  buffer.buffer[2] = StenoKeyCode::CreateRawKeyCodePress(KeyCode::F1);
  buffer.buffer[3] = StenoKeyCode('c', StenoCaseMode::NORMAL);
  buffer.buffer[4] = StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::F1);
  buffer.count = 5;

  buffer.Backspace(2);

  assert(buffer.count == 3);
  assert(buffer.buffer[0] == StenoKeyCode('a', StenoCaseMode::NORMAL));
  assert(buffer.buffer[1] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::F1));
  assert(buffer.buffer[2] ==
         StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::F1));
}
TEST_END

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "engine.h"
#include "key_code.h"
#include "segment.h"
#include "state.h"
#include "steno_key_code_buffer.h"
#include "steno_key_code_emitter.h"
#include "str.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

struct KeyCodeFunctionEntry {
  const char *name;
  bool (StenoKeyCodeBuffer::*handler)(const List<char *> &parameters);
};

constexpr KeyCodeFunctionEntry HANDLERS[] = {
    {"retro_capitalise", &StenoKeyCodeBuffer::RetroCapitalizeFunction},
    {"retro_title", &StenoKeyCodeBuffer::RetroTitleCaseFunction},
    {"retro_upper", &StenoKeyCodeBuffer::RetroUpperCaseFunction},
    {"retro_lower", &StenoKeyCodeBuffer::RetroLowerCaseFunction},
    {"retro_single_quotes", &StenoKeyCodeBuffer::RetroSingleQuotesFunction},
    {"retro_double_quotes", &StenoKeyCodeBuffer::RetroDoubleQuotesFunction},
    {"unicode", &StenoKeyCodeBuffer::UnicodeFunction},
    {"keyboard_layout", &StenoKeyCodeBuffer::KeyboardLayoutFunction},
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

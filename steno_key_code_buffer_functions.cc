//---------------------------------------------------------------------------

#include "console.h"
#include "engine.h"
#include "host_layout.h"
#include "mem.h"
#include "state.h"
#include "steno_key_code.h"
#include "steno_key_code_buffer.h"
#include "str.h"
#include "utf8_pointer.h"

//---------------------------------------------------------------------------

struct KeyCodeFunctionEntry {
  const char *name;
  bool (StenoKeyCodeBuffer::*handler)(const List<char *> &parameters);
};

// clang-format off
constexpr KeyCodeFunctionEntry HANDLERS[] = {
    {"add_translation", &StenoKeyCodeBuffer::AddTranslationFunction},
    {"console", &StenoKeyCodeBuffer::ConsoleFunction},
    {"disable_all_dictionaries", &StenoKeyCodeBuffer::DisableAllDictionariesFunction},
    {"disable_dictionary", &StenoKeyCodeBuffer::DisableDictionaryFunction},
    {"enable_all_dictionaries", &StenoKeyCodeBuffer::EnableAllDictionariesFunction},
    {"enable_dictionary", &StenoKeyCodeBuffer::EnableDictionaryFunction},
    {"host_layout", &StenoKeyCodeBuffer::HostLayoutFunction},
    {"repeat_last_character", &StenoKeyCodeBuffer::RepeatLastCharacter},
    {"repeat_last_fragment", &StenoKeyCodeBuffer::RepeatLastFragment},
    {"repeat_last_word", &StenoKeyCodeBuffer::RepeatLastWord},
    {"reset_state", &StenoKeyCodeBuffer::ResetStateFunction},
    {"retro_capitalise", &StenoKeyCodeBuffer::RetroCapitalizeFunction},
    {"retro_double_quotes", &StenoKeyCodeBuffer::RetroDoubleQuotesFunction},
    {"retro_lower", &StenoKeyCodeBuffer::RetroLowerCaseFunction},
    {"retro_replace_space", &StenoKeyCodeBuffer::RetroReplaceSpaceFunction},
    {"retro_single_quotes", &StenoKeyCodeBuffer::RetroSingleQuotesFunction},
    {"retro_surround", &StenoKeyCodeBuffer::RetroSurroundFunction},
    {"retro_title", &StenoKeyCodeBuffer::RetroTitleCaseFunction},
    {"retro_upper", &StenoKeyCodeBuffer::RetroUpperCaseFunction},
    {"set_case", &StenoKeyCodeBuffer::SetCaseFunction},
    {"set_space", &StenoKeyCodeBuffer::SetSpaceFunction},
    {"stitch", &StenoKeyCodeBuffer::StitchFunction},
    {"stitch_last_word", &StenoKeyCodeBuffer::StitchLastWordFunction},
    {"toggle_dictionary", &StenoKeyCodeBuffer::ToggleDictionaryFunction},
};
// clang-format on

//---------------------------------------------------------------------------

static bool ReadIntegerParameter(int &result, const char *p,
                                 bool allowNegative = false) {
  p = Str::ParseInteger(&result, p, allowNegative);
  return p && *p == '\0';
}

//---------------------------------------------------------------------------

// `a b {#key1} c {#key2}` with two backspaces should leave:
// `a {#key1} {#key2}
void StenoKeyCodeBuffer::Backspace(int backspaceCount) {
  StenoKeyCode *pEnd = currentOutput;
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
  currentOutput = d;
}

void StenoKeyCodeBuffer::RetroactiveCapitalize(int wordCount) {
  if (GetCount() == 0) {
    return;
  }

  StenoKeyCode *p = currentOutput - 1;
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
      --p;
    }

    --wordCount;
  }

epilog:
  lastCharacterPointer->SetCase(StenoCaseMode::TITLE_ONCE);
}

void StenoKeyCodeBuffer::RetroactiveUncapitalize(int wordCount) {
  StenoKeyCode *p = currentOutput - 1;
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
  StenoKeyCode *p = currentOutput - 1;
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
    lastCharacterPointer->SetCase(StenoCaseMode::TITLE);

    --wordCount;
  }
}

void StenoKeyCodeBuffer::RetroactiveUpperCase(int wordCount) {
  StenoKeyCode *p = currentOutput - 1;
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
  StenoKeyCode *p = currentOutput - 1;
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

void StenoKeyCodeBuffer::RetroactiveReplaceSpace(int wordCount,
                                                 const char *replacement) {
  const size_t replacementLength = Str::Length(replacement);
  StenoKeyCode *p = currentOutput;
  while (wordCount) {
    for (;;) {
      if (p <= buffer) {
        return;
      }
      --p;
      if (p->IsSpace()) {
        break;
      }
    }

    StenoKeyCode *endText = currentOutput;
    AppendText(replacement, replacementLength, StenoCaseMode::NORMAL);
    StenoKeyCode *endReplacement = currentOutput;

    // Rotate in place.
    Reverse(endText, endReplacement);
    Reverse(p, endReplacement);
    Reverse(p + (endReplacement - endText), endReplacement - 1);
    --currentOutput; // Remove space at end

    --wordCount;
  }
}

void StenoKeyCodeBuffer::RepeatLastCharacterCount(int characterCount) {
  const StenoKeyCode *pEnd = currentOutput;
  const StenoKeyCode *p = pEnd - characterCount;
  if (p < buffer) {
    p = buffer;
  }

  while (p < pEnd) {
    *currentOutput++ = *p++;
  }
}

void StenoKeyCodeBuffer::RepeatLastFragmentCount(int fragmentCount) {
  StenoKeyCode *p = currentOutput - 1;
  while (fragmentCount > 0) {
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

    --fragmentCount;
  }

  if (p->IsWhitespace()) {
    ++p;
  }

  StenoKeyCode *pEnd = currentOutput;
  while (pEnd > buffer && pEnd[-1].IsWhitespace()) {
    --pEnd;
  }
  AppendSpace();
  while (p < pEnd) {
    *currentOutput++ = *p++;
  }
}

void StenoKeyCodeBuffer::RepeatLastWordCount(int wordCount) {
  StenoKeyCode *p = currentOutput;
  while (wordCount > 0) {
    if (p == buffer) {
      return;
    }
    --p;
    while (p > buffer && p->IsWhitespace()) {
      --p;
    }

    if (p->IsWordCharacter()) {
      while (p > buffer && p[-1].IsWordCharacter()) {
        --p;
      }
    } else {
      while (p > buffer && !p[-1].IsWordCharacter() && !p[-1].IsWhitespace()) {
        --p;
      }
    }

    --wordCount;
  }

  StenoKeyCode *pEnd = currentOutput;
  while (pEnd > buffer && pEnd[-1].IsWhitespace()) {
    --pEnd;
  }
  AppendSpace();
  while (p < pEnd) {
    *currentOutput++ = *p++;
  }
}

void StenoKeyCodeBuffer::RetroactiveQuotes(int wordCount,
                                           const char *startQuote,
                                           const char *endQuote) {
  if (GetCount() == 0) {
    return;
  }

  StenoKeyCode *endQuoteBufferPointer = currentOutput;
  AppendText(endQuote, Str::Length(endQuote), StenoCaseMode::NORMAL);
  StenoKeyCode *startQuoteBufferPointer = currentOutput;
  AppendText(startQuote, Str::Length(startQuote), StenoCaseMode::NORMAL);

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
  Reverse(startQuoteBufferPointer, currentOutput);
  Reverse(p, currentOutput);
}

void StenoKeyCodeBuffer::RetroactiveSingleQuotes(int count) {
  RetroactiveQuotes(count, "'", "'");
}

void StenoKeyCodeBuffer::RetroactiveDoubleQuotes(int count) {
  RetroactiveQuotes(count, "\"", "\"");
}

void StenoKeyCodeBuffer::RetroactiveDeleteSpace() {
  // Finds the first whitespace in the buffer and removes it.
  StenoKeyCode *end = currentOutput;
  StenoKeyCode *p = end - 1;
  while (p >= buffer) {
    if (p->IsWhitespace()) {
      --currentOutput;
      memmove(p, p + 1, sizeof(StenoKeyCode) * (end - p - 1));
      return;
    }
    --p;
  }
}

// Currency takes the previous number and a template, and replaces a 'c'
// with a number formatted with a comma every 3 digits, and at least two digits
// after the decimal point.
void StenoKeyCodeBuffer::RetroactiveFormatCurrency(const char *pStart,
                                                   const char *pEnd) {
  char numberBuffer[32];
  size_t numberBufferLength = 0;
  bool hasDecimal = false;
  int integralDigits = 0;
  int decimalDigits = 0;

  StenoKeyCode *end = currentOutput;
  StenoKeyCode *p = end - 1;
  while (p >= buffer) {
    if (p->IsAsciiDigit()) {
      numberBuffer[numberBufferLength++] = p->GetUnicode();
      ++integralDigits;
    } else if (p->IsUnicode('.')) {
      if (hasDecimal) {
        break;
      }
      hasDecimal = true;
      decimalDigits = integralDigits;
      integralDigits = 0;
    } else if (!p->IsUnicode(',')) {
      break;
    }
    --p;
    if (numberBufferLength >= 31) {
      break;
    }
  }
  ++p;

  if (integralDigits == 0) {
    numberBuffer[numberBufferLength++] = '0';
    integralDigits = 1;
  }

  // Replace the buffer with the template.
  for (Utf8Pointer t = pStart; t < pEnd; ++t) {
    if (const uint32_t unicode = *t; unicode != 'c') {
      *p++ = StenoKeyCode(unicode, StenoCaseMode::NORMAL);
      continue;
    }

    size_t i = numberBufferLength;
    int remainingIntegralDigits = integralDigits;
    while (i != 0) {
      const char c = numberBuffer[--i];
      *p++ = StenoKeyCode(c, StenoCaseMode::NORMAL);
      --remainingIntegralDigits;
      if (remainingIntegralDigits == 0) {
        // Do decimal part.
        if (hasDecimal) {
          *p++ = StenoKeyCode('.', StenoCaseMode::NORMAL);
          while (i != 0) {
            *p++ = StenoKeyCode(numberBuffer[--i], StenoCaseMode::NORMAL);
          }
          for (size_t d = decimalDigits; d < 2; ++d) {
            *p++ = StenoKeyCode('0', StenoCaseMode::NORMAL);
          }
          break;
        }
      } else if (remainingIntegralDigits % 3 == 0) {
        *p++ = StenoKeyCode(',', StenoCaseMode::NORMAL);
      }
    }
  }
  state.isGlue = false;
  currentOutput = p;
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::ProcessFunction(const List<char *> &parameters) {
  size_t left = 0;
  size_t right = sizeof(HANDLERS) / sizeof(*HANDLERS);

  while (left < right) {
    const size_t mid = (left + right) >> 1;

    const KeyCodeFunctionEntry &entry = HANDLERS[mid];
    const int compare = Str::Compare(parameters[0], entry.name);
    if (compare < 0) {
      right = mid;
    } else if (compare > 0) {
      left = mid + 1;
    } else {
      return (this->*entry.handler)(parameters);
    }
  }
  return false;
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::AddTranslationFunction(
    const List<char *> &parameters) {
  free(addTranslationText);
  addTranslationText = nullptr;
  if (!executeSideEffects) {
    return true;
  }

  launchAddTranslation = true;

  if (parameters.GetCount() >= 2) {
    BufferWriter unescaped;
    const char *p = parameters[1];
    while (*p) {
      int c = *p++;
      if (c == '\\') {
        c = *p++;
        if (c == '\0') {
          break;
        }
        switch (c) {
        case ':':
        case '{':
        case '}':
        case '\\':
          break;
        default:
          unescaped.WriteByte('\\');
        }
      }

      unescaped.WriteByte(c);
    }
    unescaped.WriteByte('\0');
    addTranslationText = Str::Trim(unescaped.GetBuffer());
    if (*addTranslationText == '\0') {
      free(addTranslationText);
      addTranslationText = nullptr;
    }
  }

  return true;
}

bool StenoKeyCodeBuffer::EnableDictionaryFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  doResetState = true;
  return rootDictionary->EnableDictionary(parameters[1]);
}

bool StenoKeyCodeBuffer::DisableDictionaryFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  doResetState = true;
  return rootDictionary->DisableDictionary(parameters[1]);
}

bool StenoKeyCodeBuffer::EnableAllDictionariesFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 1) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  doResetState = true;
  rootDictionary->EnableAllDictionaries();
  return true;
}

bool StenoKeyCodeBuffer::DisableAllDictionariesFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 1) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  doResetState = true;
  rootDictionary->DisableAllDictionaries();
  return true;
}

bool StenoKeyCodeBuffer::ToggleDictionaryFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  doResetState = true;
  return rootDictionary->ToggleDictionary(parameters[1]);
}

bool StenoKeyCodeBuffer::CountHandler(void (StenoKeyCodeBuffer::*handler)(int),
                                      const List<char *> &parameters) {

  int count;
  switch (parameters.GetCount()) {
  case 1:
    count = 1;
    break;
  case 2:
    if (!ReadIntegerParameter(count, parameters[1])) {
      return false;
    }
    break;
  default:
    return false;
  }

  (this->*handler)(count);
  return true;
}

bool StenoKeyCodeBuffer::RepeatLastCharacter(const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RepeatLastCharacterCount,
                      parameters);
}

bool StenoKeyCodeBuffer::RepeatLastFragment(const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RepeatLastFragmentCount, parameters);
}

bool StenoKeyCodeBuffer::RepeatLastWord(const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RepeatLastWordCount, parameters);
}

bool StenoKeyCodeBuffer::RetroCapitalizeFunction(
    const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RetroactiveCapitalize, parameters);
}

bool StenoKeyCodeBuffer::RetroTitleCaseFunction(
    const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RetroactiveTitleCase, parameters);
}

bool StenoKeyCodeBuffer::RetroUpperCaseFunction(
    const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RetroactiveUpperCase, parameters);
}

bool StenoKeyCodeBuffer::RetroLowerCaseFunction(
    const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RetroactiveLowerCase, parameters);
}

bool StenoKeyCodeBuffer::RetroReplaceSpaceFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 3) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveReplaceSpace(wordCount, parameters[2]);
  return true;
}

bool StenoKeyCodeBuffer::RetroSingleQuotesFunction(
    const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RetroactiveSingleQuotes, parameters);
}

bool StenoKeyCodeBuffer::RetroSurroundFunction(const List<char *> &parameters) {
  if (parameters.GetCount() != 4) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  RetroactiveQuotes(wordCount, parameters[2], parameters[3]);
  return true;
}

bool StenoKeyCodeBuffer::RetroDoubleQuotesFunction(
    const List<char *> &parameters) {
  return CountHandler(&StenoKeyCodeBuffer::RetroactiveDoubleQuotes, parameters);
}

bool StenoKeyCodeBuffer::SetCaseFunction(const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  if (Str::Eq(parameters[1], "normal")) {
    // This is needed because orthographic suffixes can make the case mode
    // take on the overriden case mode. This is not a problem for the other
    // cases as the override will kick in.
    switch (state.caseMode) {
    case StenoCaseMode::NORMAL:
    case StenoCaseMode::LOWER_ONCE:
    case StenoCaseMode::UPPER_ONCE:
    case StenoCaseMode::TITLE_ONCE:
      // Don't do anything
      break;
    default:
      state.caseMode = StenoCaseMode::NORMAL;
      break;
    }
    state.overrideCaseMode = StenoCaseMode::NORMAL;
    return true;
  }

  struct StenoCaseModeName {
    const char *key;
    StenoCaseMode value;
  };
  static constexpr StenoCaseModeName names[] = {
      {.key = "lower", .value = StenoCaseMode::LOWER},
      {.key = "upper", .value = StenoCaseMode::UPPER},
      {.key = "title", .value = StenoCaseMode::TITLE},
      {.key = "camel", .value = StenoCaseMode::CAMEL},
  };

  for (const StenoCaseModeName &name : names) {
    if (Str::Eq(parameters[1], name.key)) {
      state.overrideCaseMode = name.value;
      return true;
    }
  }

  return false;
}

bool StenoKeyCodeBuffer::SetSpaceFunction(const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  state.SetSpace(parameters[1]);
  return true;
}

bool StenoKeyCodeBuffer::StitchFunction(const List<char *> &parameters) {
  if (parameters.GetCount() < 2) {
    return false;
  }
  if (wasLastActionAStitch) {
    const char *delimiter = parameters.GetCount() >= 3 ? parameters[2] : "-";
    AppendText(delimiter, Str::Length(delimiter), state.caseMode);
  } else if (!state.joinNext) {
    AppendText(state.GetSpace(), state.spaceLength, StenoCaseMode::NORMAL);
  }
  const char *text = parameters[1];
  AppendText(text, Str::Length(text), state.caseMode);
  state.joinNext = false;
  state.isGlue = false;
  state.isManualStateChange = false;
  state.caseMode = state.GetNextWordCaseMode();
  wasLastActionAStitch = true;
  return true;
}

bool StenoKeyCodeBuffer::StitchLastWordFunction(
    const List<char *> &parameters) {
  if (parameters.GetCount() != 3) {
    return false;
  }

  int wordCount;
  if (!ReadIntegerParameter(wordCount, parameters[1])) {
    return false;
  }

  const char *delimiter = parameters[2];
  if (*delimiter == '\0') {
    return true;
  }

  StenoKeyCode *start = currentOutput;
  AppendText(delimiter, Str::Length(delimiter), StenoCaseMode::NORMAL);
  StenoKeyCode *end = currentOutput;
  if (end <= start) {
    return true;
  }

  const size_t delimiterLength = end - start;
  StenoKeyCode delimiterKeyCodes[delimiterLength];
  Mem::Copy(delimiterKeyCodes, start, sizeof(StenoKeyCode) * delimiterLength);
  currentOutput = start;

  size_t delimiterNeededCount = 0;

  StenoKeyCode *p = currentOutput - 1;

  for (size_t wc = 0; wc < wordCount; ++wc, --p) {
    for (; p > buffer; --p) {
      if (!p->IsWhitespace()) {
        break;
      }
    }

    while (p > buffer && !p[-1].IsWhitespace()) {
      ++delimiterNeededCount;
      --p;
    }
  }

  p = currentOutput - 1;
  currentOutput += delimiterNeededCount * delimiterLength;
  StenoKeyCode *output = p + delimiterNeededCount * delimiterLength;

  for (size_t wc = 0; wc < wordCount; ++wc) {
    while (p > buffer) {
      if (!p->IsWhitespace()) {
        break;
      }
      *output-- = *p--;
    }

    while (p > buffer && !p[-1].IsWhitespace()) {
      *output = *p--;
      output -= delimiterLength;
      Mem::Copy(output, delimiterKeyCodes,
                sizeof(StenoKeyCode) * delimiterLength);
      --output;
    }

    if (p >= buffer) {
      *output-- = *p--;
    }
  }

  return true;
}

bool StenoKeyCodeBuffer::ResetStateFunction(const List<char *> &) {
  if (!executeSideEffects) {
    return true;
  }

  doResetState = true;
  return true;
}

//---------------------------------------------------------------------------

bool StenoKeyCodeBuffer::HostLayoutFunction(const List<char *> &parameters) {
  if (parameters.GetCount() != 2) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  return HostLayouts::SetActiveLayout(parameters[1]);
}

bool StenoKeyCodeBuffer::ConsoleFunction(const List<char *> &parameters) {
  if (parameters.GetCount() > 2) {
    return false;
  }

  if (!executeSideEffects) {
    return true;
  }

  if (parameters.GetCount() == 1) {
    launchConsole = true;
  } else {
    Console::RunCommand(parameters[1], *ConsoleWriter::GetActiveWriter());
  }

  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"

void AssertBufferContent(const StenoKeyCodeBuffer &buffer,
                         const char *expected) {
  char *text = buffer.ToString();
  assert(Str::Eq(text, expected));
  free(text);
}

TEST_BEGIN("StenoKeyCodeBuffer: Backspace() should give expected results") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  *buffer.currentOutput++ = StenoKeyCode('a', StenoCaseMode::NORMAL);
  *buffer.currentOutput++ = StenoKeyCode('b', StenoCaseMode::NORMAL);
  *buffer.currentOutput++ = StenoKeyCode::CreateRawKeyCodePress(KeyCode::F1);
  *buffer.currentOutput++ = StenoKeyCode('c', StenoCaseMode::NORMAL);
  *buffer.currentOutput++ = StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::F1);

  buffer.Backspace(2);

  assert(buffer.GetCount() == 3);
  assert(buffer.buffer[0] == StenoKeyCode('a', StenoCaseMode::NORMAL));
  assert(buffer.buffer[1] == StenoKeyCode::CreateRawKeyCodePress(KeyCode::F1));
  assert(buffer.buffer[2] ==
         StenoKeyCode::CreateRawKeyCodeRelease(KeyCode::F1));
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: RetroReplaceSpace") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RetroactiveReplaceSpace(1, "");
  AssertBufferContent(buffer, "abc");
  ;
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: RetroReplaceSpace _") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RetroactiveReplaceSpace(1, "_");
  AssertBufferContent(buffer, "ab_c");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: RetroReplaceSpace <>") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RetroactiveReplaceSpace(1, "<>");
  AssertBufferContent(buffer, "ab<>c");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: :repeat_last_character:1") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RepeatLastCharacterCount(1);
  AssertBufferContent(buffer, "ab cc");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: :repeat_last_character:3") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RepeatLastCharacterCount(3);
  AssertBufferContent(buffer, "ab cb c");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: :repeat_last_fragment:1") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RepeatLastFragmentCount(1);
  AssertBufferContent(buffer, "ab c c");
  ;
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: :repeat_last_fragment:2") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab: (c)", 7, StenoCaseMode::NORMAL);
  buffer.RepeatLastFragmentCount(2);
  AssertBufferContent(buffer, "ab: (c) ab: (c)");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: :repeat_last_word:1") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab c", 4, StenoCaseMode::NORMAL);
  buffer.RepeatLastWordCount(1);
  AssertBufferContent(buffer, "ab c c");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: :repeat_last_word:2") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab: (c)", 7, StenoCaseMode::NORMAL);
  buffer.RepeatLastWordCount(2);
  AssertBufferContent(buffer, "ab: (c) c)");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: stitch_last_word:1:-") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("test test", 9, StenoCaseMode::NORMAL);

  List<const char *> parameters;
  parameters.Add("");
  parameters.Add("1");
  parameters.Add("-");
  buffer.StitchLastWordFunction(*(const List<char *> *)&parameters);

  AssertBufferContent(buffer, "test t-e-s-t");
}
TEST_END

TEST_BEGIN("StenoKeyCodeBuffer: stitch_last_word:2:->") {
  StenoKeyCodeBuffer buffer;
  buffer.Reset();
  buffer.AppendText("ab cde fg", 9, StenoCaseMode::NORMAL);

  List<const char *> parameters;
  parameters.Add("");
  parameters.Add("2");
  parameters.Add("->");
  buffer.StitchLastWordFunction(*(const List<char *> *)&parameters);

  AssertBufferContent(buffer, "ab c->d->e f->g");
}
TEST_END

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "steno_key_code.h"
#include "steno_key_code_data.h"
#include <assert.h>

//---------------------------------------------------------------------------

static uint32_t Lookup(uint32_t c, const StenoKeyPair *data, size_t length) {
  size_t left = 0;
  size_t right = length;
  while (left < right) {
    size_t mid = (left + right) / 2;
    uint32_t midKey = data[mid].key;

    if (c < midKey) {
      right = mid;
    } else if (c > midKey) {
      left = mid + 1;
    } else {
      return data[mid].value;
    }
  }
  return c;
}

static uint32_t Lookup(uint32_t c, const StenoKeyRange *data, size_t length) {
  size_t left = 0;
  size_t right = length;
  while (left < right) {
    size_t mid = (left + right) / 2;
    if (c < data[mid].begin) {
      right = mid;
    } else if (c > data[mid].end) {
      left = mid + 1;
    } else {
      return c != data[mid].end;
    }
  }
  return c;
}

//---------------------------------------------------------------------------

uint32_t ToUpper(uint32_t c) {
  if (c < 128) {
    if (c < 'a' || c > 'z') {
      return c;
    }
    return c + 'A' - 'a';
  }
  return Lookup(c, UPPER_DATA, sizeof(UPPER_DATA) / sizeof(*UPPER_DATA));
}

uint32_t ToLower(uint32_t c) {
  if (c < 128) {
    if (c < 'A' || c > 'Z') {
      return c;
    }
    return c + 'a' - 'A';
  }
  return Lookup(c, LOWER_DATA, sizeof(LOWER_DATA) / sizeof(*LOWER_DATA));
}

bool IsAsciiDigit(uint32_t c) { return '0' <= c && c <= '9'; }

bool IsLetter(uint32_t c) {
  if (c < 128) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
  }
  return Lookup(c, LETTER_DATA, sizeof(LETTER_DATA) / sizeof(*LETTER_DATA));
}

bool IsWhitespace(uint32_t c) {
  switch (c) {
  case '\0':
  case '\n':
  case '\r':
  case '\t':
  case '\f':
  case '\v':
  case ' ':
    return true;
  default:
    return false;
  }
}

bool IsWordCharacter(uint32_t c) {
  return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9');
}

//---------------------------------------------------------------------------

uint32_t StenoKeyCode::ResolveUnicode() const {
  switch (caseMode) {
  case StenoCaseMode::NORMAL:
    return unicode;
  case StenoCaseMode::LOWER:
  case StenoCaseMode::LOWER_ONCE:
    return ::ToLower(unicode);
  case StenoCaseMode::UPPER:
  case StenoCaseMode::UPPER_ONCE:
  case StenoCaseMode::TITLE:
  case StenoCaseMode::TITLE_ONCE:
    return ::ToUpper(unicode);
  default:
    __builtin_unreachable();
  }
  return unicode;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "unicode.h"
#include "unicode_data.h"
#include <assert.h>

//---------------------------------------------------------------------------

static uint32_t Lookup(uint32_t c, const UnicodePair *data, size_t length) {
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

static bool Lookup(uint32_t c, const UnicodeRange *data, size_t length) {
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
  return false;
}

//---------------------------------------------------------------------------

uint32_t Unicode::ToUpper(uint32_t c) {
  if (c < 128) {
    if (c < 'a' || c > 'z') {
      return c;
    }
    return c + 'A' - 'a';
  }
  return Lookup(c, UPPER_DATA, sizeof(UPPER_DATA) / sizeof(*UPPER_DATA));
}

uint32_t Unicode::ToLower(uint32_t c) {
  if (c < 128) {
    if (c < 'A' || c > 'Z') {
      return c;
    }
    return c + 'a' - 'A';
  }
  return Lookup(c, LOWER_DATA, sizeof(LOWER_DATA) / sizeof(*LOWER_DATA));
}

bool Unicode::IsLetter(uint32_t c) {
  if (c < 128) {
    c |= 0x20;
    return 'a' <= c && c <= 'z';
  }
  return Lookup(c, LETTER_DATA, sizeof(LETTER_DATA) / sizeof(*LETTER_DATA));
}

#if JAVELIN_CPU_CORTEX_M0 || JAVELIN_CPU_CORTEX_M4
#else
bool Unicode::IsWhitespace(uint32_t c) {
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
#endif

bool Unicode::IsWordCharacter(uint32_t c) {
  if (c == '_') {
    return true;
  }
  if ('0' <= c && c <= '9') {
    return true;
  }
  c |= 0x20;
  return 'a' <= c && c <= 'z';
}

//---------------------------------------------------------------------------

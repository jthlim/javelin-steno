//---------------------------------------------------------------------------

#include "str.h"
#include "unicode.h"
#include "writer.h"
#include <stdarg.h>

//---------------------------------------------------------------------------

bool Str::IsSpace(const char *p) {
  if (*p != '{') {
    return false;
  }

  if (p[1] != '^') {
    return false;
  }

  p += 2;
  if (p[0] == '~' && p[1] == '|') {
    p += 2;
  }
  if (Unicode::IsWhitespace(*p)) {
    ++p;
  } else if (p[0] == '\\' && (p[1] == 'n' || p[1] == 'r' || p[1] == 't' ||
                              p[1] == 'f' || p[1] == 'v')) {
    p += 2;
  } else {
    return false;
  }
  return p[0] == '^' && p[1] == '}';
}

bool Str::IsWordSeparator(const char *p) {
  for (;;) {
    if (p[0] == '{' && p[1] == '}') {
      p += 2;
      continue;
    }

    if (p[0] == '{') {
      if (p[1] == '^') {
        if (p[2] == '}') {
          p += 3;
        } else {
          p += 2;
        }
      } else {
        p += 1;
      }
    }
    break;
  }

  const char *pEnd = p + Str::Length(p) - 2;
  for (;;) {
    if (pEnd[1] != '}') {
      return false;
    }
    if (pEnd[0] == '{') {
      pEnd += 2;
      continue;
    }

    if (pEnd[0] != '^') {
      return false;
    }
    if (pEnd[-1] == '{') {
      pEnd--;
    }
    break;
  }
  if (p[0] == '~' && p[1] == '|') {
    p += 2;
  }

  if (p >= pEnd) {
    return false;
  }

  if (p + 1 == pEnd && p[0] == '-') {
    return false;
  }

  while (p < pEnd) {
    switch (int c = *p++; c) {
    case '\\':
      switch (c = *p++; c) {
      case '0':
      case 'e':
      case 'n':
      case 'r':
      case 't':
      case 'f':
      case 'v':
      case ' ':
        continue;
      }

      [[fallthrough]];

    default:
      if (Unicode::IsWordCharacter(c)) {
        return false;
      }
      break;
    }
  }

  return true;
}

char *Str::Asprintf(const char *p, ...) {
  va_list args;
  va_start(args, p);

  BufferWriter bufferWriter;
  bufferWriter.Vprintf(p, args);
  va_end(args);

  return bufferWriter.TerminateStringAndAdoptBuffer();
}

size_t Str::Sprintf(char *target, const char *p, ...) {
  va_list args;
  va_start(args, p);

  MemoryWriter memoryWriter(target);
  memoryWriter.Vprintf(p, args);
  const size_t result = (char *)memoryWriter.GetTarget() - target;
  memoryWriter.WriteByte('\0');

  va_end(args);
  return result;
}

bool Str::IsFingerSpellingCommand(const char *p) {
  while (*p) {
    const int c = *p++;
    if (c == ' ') [[unlikely]] {
      continue;
    }
    if (c == '{') [[unlikely]] {
      if (*p != '&') {
        return false;
      }
      ++p;
      for (;;) {
        const int c = *p++;
        if (c == '\0') {
          return false;
        }
        if (c == '\\' && *p != '\0') {
          ++p;
        } else if (c == '}') {
          if (*p == '\0') {
            return true;
          }
          break;
        }
      }
    } else {
      return false;
    }
  }

  return false;
}

bool Str::IsJoinPrevious(const char *p) {
  return p[0] == '{' && p[1] == '^' && !Contains(p + 2, '\n');
}

bool Str::ContainsKeyCode(const char *p) {
  for (;;) {
    const int c = *p++;
    if (c == '\0') [[unlikely]] {
      return false;
    }

    if (c == '\\') [[unlikely]] {
      if (*p == '\0') {
        return false;
      }
      ++p;
      continue;
    }

    if (c != '{') [[likely]] {
      continue;
    }

    if (*p == '#') [[unlikely]] {
      return true;
    }
  }
}

[[gnu::noinline]] char *Str::Join(const char *const *data, size_t n) {
  char *result = (char *)malloc(64);
  char *d = result;
  char *guard = result + 64;

  do {
    const char *p = *data++;
    while (*p) {
      *d++ = *p++;
      if (d == guard) {
        // Expand the buffer.
        const size_t length = guard - result;
        const size_t newLength = length * 2;
        result = (char *)realloc(result, newLength);
        guard = result + newLength;
        d = result + length;
      }
    }
  } while (--n);
  *d++ = '\0';

  return result;
}

[[gnu::noinline]]
char *Str::DupN(const char *p, size_t length) {
  char *buffer = (char *)malloc(length + 1);
  buffer[length] = '\0';
  return (char *)memcpy(buffer, p, length);
}

[[gnu::noinline]]
char *Str::Dup(const char *p) {
  const size_t length = Length(p);
  char *buffer = (char *)malloc(length + 1);
  buffer[length] = '\0';
  return (char *)memcpy(buffer, p, length);
}

char *Str::CreateEmpty() {
  char *buffer = (char *)malloc(1);
  buffer[0] = '\0';
  return buffer;
}

bool Str::Eq(const char *a, const char *b, size_t bLength) {
  while (bLength) {
    if (*a++ != *b++) {
      return false;
    }
    --bLength;
  }
  return *a == '\0';
}

bool Str::HasPrefix(const char *p, const char *prefix) {
  for (;;) {
    const int c = *prefix++;
    if (c == '\0') {
      return true;
    }
    if (c != *p++) {
      return false;
    }
  }
}

bool Str::HasPrefixWord(const char *p, const char *prefix) {
  for (;;) {
    const int c = *prefix++;
    if (c == '\0') {
      return *p == '\0' || *p == ' ';
    }
    if (c != *p++) {
      return false;
    }
  }
}

bool Str::HasSuffix(const char *p, const char *suffix) {
  const size_t length = Length(p);
  const size_t suffixLength = Length(suffix);
  if (suffixLength > length) {
    return false;
  }

  return Eq(p + length - suffixLength, suffix, suffixLength);
}

bool Str::IgnoreCaseEq(const char *a, const char *b, size_t bLength) {
  while (bLength) {
    const uint32_t ca = *(uint8_t *)a++;
    const uint32_t cb = *(uint8_t *)b++;
    if (Unicode::ToLower(ca) != cb) {
      return false;
    }
    --bLength;
  }
  return *a == '\0';
}

char *Str::Trim(const char *data) {
  const char *start = data;
  while (*start && *start == ' ') {
    ++start;
  }
  const char *p = start;
  const char *end = start;
  while (*p) {
    if (*p++ != ' ') {
      end = p;
    }
  }

  return DupN(start, end - start);
}

char *Str::WriteJson(char *p, const char *text) {
  while (*text) {
    const int c = *text++;
    if (c < 32) [[unlikely]] {
      switch (c) {
      case '\f':
        *p++ = '\\';
        *p++ = 'f';
        continue;

      case '\b':
        *p++ = '\\';
        *p++ = 'b';
        continue;

      case '\r':
        *p++ = '\\';
        *p++ = 'r';
        continue;

      case '\n':
        *p++ = '\\';
        *p++ = 'n';
        continue;

      case '\t':
        *p++ = '\\';
        *p++ = 't';
        continue;
      }
      continue;
    } else if (c == '\\' || c == '\"') [[unlikely]] {
      *p++ = '\\';
    }
    *p++ = c;
  }
  return p;
}

const char *Str::ParseInteger(int *result, const char *p, bool allowNegative) {
  int sign = 1;
  if (allowNegative && *p == '-') {
    sign = -1;
    ++p;
  }

  int value = 0;
  if (p[0] == '0' && p[1] == 'x' && Unicode::GetHexValue(p[2]) != -1) {
    p += 2;
    for (;;) {
      const int c = Unicode::GetHexValue(*p);
      if (c == -1) {
        break;
      }
      value = 16 * value + c;
      ++p;
    }
  } else if (p[0] == '0' && p[1] == 'b' && Unicode::IsBinaryDigit(p[2])) {
    p += 2;
    for (;;) {
      const int c = Unicode::GetBinaryValue(*p);
      if (c == -1) {
        break;
      }
      value = 2 * value + c;
      ++p;
    }
  } else if (Unicode::IsAsciiDigit(*p)) {
    for (;;) {
      const int c = Unicode::GetDecimalValue(*p);
      if (c == -1) {
        break;
      }
      value = 10 * value + c;
      ++p;
    }
  } else {
    return nullptr;
  }

  *result = sign * value;

  return p;
}

int Str::ParseHexDigits(const char *p, size_t n) {
  int result = 0;
  for (size_t i = 0; i < n; ++i) {
    const int c = Unicode::GetHexValue(p[i]);
    if (c == -1) {
      return -1;
    }
    result = 16 * result + c;
  }
  return result;
}

const char *Str::AdvanceToWordCharacter(const char *p) {
  if (!p) {
    return nullptr;
  }

  for (;;) {
    if (*p == '\0') {
      return nullptr;
    }
    if (Unicode::IsWordCharacter(*p)) {
      return p;
    }
    ++p;
  }
}

const char *Str::AdvanceToNonWordCharacter(const char *p) {
  for (;;) {
    if (!Unicode::IsWordCharacter(*p)) {
      return p;
    }
    ++p;
  }
}

//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("Str::ShortEq returns correct results") {
  assert(Str::ShortEq("", ""));
  assert(Str::ShortEq("abc", "abc"));
  assert(!Str::ShortEq("abc", "ade"));
  assert(!Str::ShortEq("abc", "abcde"));
  assert(!Str::ShortEq("abcde", "abc"));
}
TEST_END

TEST_BEGIN("Str::HasPrefix returns correct results") {
  assert(Str::HasPrefix("abcd", "ab"));
  assert(Str::HasPrefix("abcd", "cd") == false);
  assert(Str::HasPrefix("abcd", "") == true);
  assert(Str::HasPrefix("abcd", "abcde") == false);
}
TEST_END

TEST_BEGIN("Str::HasSuffix returns correct results") {
  assert(Str::HasSuffix("abcd", "cd"));
  assert(Str::HasSuffix("abcd", "de") == false);
  assert(Str::HasSuffix("abcd", "") == true);
  assert(Str::HasSuffix("abcd", "abcde") == false);
}
TEST_END

static bool TestTrim(const char *input, const char *expected) {
  char *output = Str::Trim(input);
  const bool result = Str::Eq(output, expected);
  free(output);
  return result;
}

TEST_BEGIN("Str::Trim returns correct results") {
  assert(TestTrim("", ""));
  assert(TestTrim("  a", "a"));
  assert(TestTrim("  ab", "ab"));
  assert(TestTrim("ab ", "ab"));
  assert(TestTrim("ab  ", "ab"));
  assert(TestTrim("  ab  ", "ab"));
}
TEST_END

TEST_BEGIN("Str::IsSpace returns correct results") {
  assert(Str::IsSpace("{^ ^}"));
  assert(Str::IsSpace("{^~|\n^}"));
  assert(Str::IsSpace("{^\n^}"));
  assert(Str::IsSpace("{^\\n^}"));
}
TEST_END

TEST_BEGIN("Str::IsWordSeparator returns correct results") {
  assert(Str::IsWordSeparator("{^@^}"));
  assert(Str::IsWordSeparator("{^ ^}"));
  assert(!Str::IsWordSeparator("{^-^}"));
  assert(Str::IsWordSeparator("{}{^@^}"));
  assert(!Str::IsWordSeparator("{^-^}"));
  assert(Str::IsWordSeparator("{}@{^}"));
  assert(!Str::IsWordSeparator("{}-{^}"));
}
TEST_END

TEST_BEGIN("Str::IsFingerSpellingCommand returns correct results") {
  assert(Str::IsFingerSpellingCommand("{&T}"));
  assert(!Str::IsFingerSpellingCommand("{&T"));
  assert(!Str::IsFingerSpellingCommand("{T}"));
  assert(!Str::IsFingerSpellingCommand("T"));
}
TEST_END

//---------------------------------------------------------------------------

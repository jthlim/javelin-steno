//---------------------------------------------------------------------------

#include "str.h"
#include "hint.h"
#include "unicode.h"
#include "writer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
  if (*p == ' ' || *p == '\n') {
    ++p;
  } else if (p[0] == '\\' && p[1] == 'n') {
    p += 2;
  } else {
    return false;
  }
  return p[0] == '^' && p[1] == '}';
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
  bool result = false;

  while (*p) {
    int c = *p++;
    if (c == ' ') {
      continue;
    }
    result = false;
    if (c == '{') {
      if (*p == '&') {
        p++;
        result = true;
      }
      while (*p) {
        int c = *p++;
        if (c == '}') {
          break;
        }
      }
    }
  }

  return result;
}

bool Str::IsJoinPrevious(const char *p) {
  return p[0] == '{' && p[1] == '^' && !Str::Contains(p + 2, '\n');
}

bool Str::ContainsKeyCode(const char *p) {
  while (*p) {
    int c = *p++;
    if (c == '{') {
      if (*p == '#') {
        return true;
      }
    }
  }

  return false;
}

__attribute__((noinline)) char *Str::Join(const char *const *data, size_t n) {
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

char *Str::DupN(const char *p, size_t length) {
  char *buffer = (char *)malloc(length + 1);
  buffer[length] = '\0';
  return (char *)memcpy(buffer, p, length);
}

char *Str::Dup(const char *p) {
  const size_t length = strlen(p);
  char *buffer = (char *)malloc(length + 1);
  buffer[length] = '\0';
  return (char *)memcpy(buffer, p, length);
}

bool Str::HasPrefix(const char *p, const char *prefix) {
  for (;;) {
    if (*prefix == '\0') {
      return true;
    }
    if (*prefix != *p) {
      return false;
    }
    ++prefix;
    ++p;
  }
}

bool Str::HasSuffix(const char *p, const char *suffix) {
  const size_t length = Length(p);
  const size_t suffixLength = Length(suffix);
  if (suffixLength > length) {
    return false;
  }

  return Str::Eq(p + length - suffixLength, suffix);
}

char *Str::Trim(const char *data) {
  const char *start = data;
  while (*start && Unicode::IsWhitespace(*start)) {
    ++start;
  }
  const char *p = start;
  const char *end = start;
  while (*p) {
    if (!Unicode::IsWhitespace(*p++)) {
      end = p;
    }
  }

  return Str::DupN(start, end - start);
}

char *Str::WriteJson(char *p, const char *text) {
  while (*text) {
    int c = *text++;
    if (JAVELIN_UNLIKELY(c < 32)) {
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
    } else if (JAVELIN_UNLIKELY(c == '\\' || c == '\"')) {
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

  if (*p < '0' || *p > '9') {
    return nullptr;
  }

  int value = 0;
  do {
    value = 10 * value + (*p++ - '0');
  } while ('0' <= *p && *p <= '9');
  *result = sign * value;

  return p;
}

//---------------------------------------------------------------------------

#include "unit_test.h"

TEST_BEGIN("Str::Suffix returns correct results") {
  assert(Str::HasSuffix("abcd", "cd"));
  assert(Str::HasSuffix("abcd", "de") == false);
  assert(Str::HasSuffix("abcd", "") == true);
  assert(Str::HasSuffix("abcd", "abcde") == false);
}
TEST_END

static bool TestTrim(const char *input, const char *expected) {
  char *output = Str::Trim(input);
  bool result = Str::Eq(output, expected);
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

//---------------------------------------------------------------------------

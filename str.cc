//---------------------------------------------------------------------------

#include "str.h"
#include "hint.h"
#include "writer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

char *Str::Asprintf(const char *p, ...) {
  va_list args;
  va_start(args, p);

  BufferWriter bufferWriter;
  bufferWriter.Vprintf(p, args);
  va_end(args);

  bufferWriter.WriteByte('\0');
  return bufferWriter.AdoptBuffer();
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
  size_t length = strlen(p);
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

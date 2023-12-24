//---------------------------------------------------------------------------

#include "str.h"
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
  size_t result = (char *)memoryWriter.GetTarget() - target;
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
  return p[0] == '{' && p[1] == '^' && !strchr(p, '\n');
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

char *Str::Join(const char *p, ...) {
  va_list v;
  va_start(v, p);

  char *result = (char *)malloc(64);
  char *d = result;
  char *guard = result + 64;

  do {
    while (*p) {
      *d++ = *p++;
      if (d == guard) {
        // Expand the buffer.
        size_t length = guard - result;
        size_t newLength = length * 2;
        char *newResult = (char *)malloc(newLength);
        memcpy(newResult, result, length);
        free(result);
        result = newResult;
        guard = result + newLength;
        d = result + length;
      }
    }

    p = va_arg(v, const char *);
  } while (p);
  *d++ = '\0';

  va_end(v);

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
    switch (int c = *text++; c) {
    case '\f':
      *p++ = '\\';
      *p++ = 'f';
      break;

    case '\b':
      *p++ = '\\';
      *p++ = 'b';
      break;

    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      break;

    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      break;

    case '\t':
      *p++ = '\\';
      *p++ = 't';
      break;

    case '\\':
    case '\"':
      *p++ = '\\';
      [[fallthrough]];
    default:
      *p++ = c;
      break;
    }
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

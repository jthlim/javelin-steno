//---------------------------------------------------------------------------

#include "str.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

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

char *Str::Asprintf(const char *p, ...) {
  va_list v;
  va_start(v, p);

  size_t length = vsnprintf(nullptr, 0, p, v) + 1;
  char *buffer = (char *)malloc(length);
  vsnprintf(buffer, length, p, v);
  va_end(v);

  return buffer;
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
      ++text;
      break;

    case '\b':
      *p++ = '\\';
      *p++ = 'b';
      ++text;
      break;

    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      ++text;
      break;

    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      ++text;
      break;

    case '\t':
      *p++ = '\\';
      *p++ = 't';
      ++text;
      break;

    case '\\':
    case '\"':
      *p++ = '\\';
      [[clang::fallthrough]];
    default:
      *p++ = c;
      break;
    }
  }
  return p;
}

#if JAVELIN_ASSEMBLER_THUMB2
extern "C" __attribute((naked)) size_t strlen(const char *p) {
  // spell-checker: disable
  asm volatile(R"(
    push  {r0, r4}

    lsl   r1, r0, #30
    beq   2f            // p is aligned

  1:
    ldrb  r3, [r0]
    cmp   r3, #0
    beq   5f            // terminating '\0' found

    add   r0, #1
    lsl   r1, r0, #30
    bne   1b            // Loop until aligned

  2:                    // p is aligned
    ldr   r2, =#0x01010101
    lsl   r3, r2, #7

  3:                    // Process 4 bytes
    ldmia r0!, {r1}
    sub   r4, r1, r2
    bic   r4, r1
    and   r4, r3
    beq   3b

  4:                    // Trailer of loop.
    sub   r0, #4
    lsr   r4, #8
    bcs   5f

    add   r0, #1
    lsr   r4, #8
    bcs   5f

    add   r0, #1
    lsr   r4, #8
    bcs   5f

    add   r0, #1

  5:
    pop   {r1, r4}
    sub   r0, r1
    bx    lr
  )");
  // spell-checker: enable
}
#endif

//---------------------------------------------------------------------------

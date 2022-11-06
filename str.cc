//---------------------------------------------------------------------------

#include "str.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

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
  return strncmp(prefix, p, strlen(prefix)) == 0;
}

char *Str::WriteJson(char *p, const char *text) {
  while (*text) {
    switch (*text) {
    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      text += 2;
      break;

    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      text += 2;
      break;

    case '\t':
      *p++ = '\\';
      *p++ = 't';
      text += 2;
      break;

    case '\\':
    case '\"':
      *p++ = '\\';
      [[clang::fallthrough]];
    default:
      *p++ = *text++;
      break;
    }
  }
  return p;
}

//---------------------------------------------------------------------------

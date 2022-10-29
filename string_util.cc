//---------------------------------------------------------------------------

#include "string_util.h"
#include <stdarg.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

char *rasprintf(const char *p, ...) {
  va_list v;
  va_start(v, p);

  size_t length = vsnprintf(nullptr, 0, p, v) + 1;
  char *buffer = (char *)malloc(length);
  vsnprintf(buffer, length, p, v);
  va_end(v);

  return buffer;
}

__attribute__((weak)) char *strndup(const char *p, size_t length) {
  char *buffer = (char *)malloc(length + 1);
  buffer[length] = '\0';
  return (char *)memcpy(buffer, p, length);
}

bool str_has_prefix(const char *p, const char *prefix) {
  return strncmp(prefix, p, strlen(prefix)) == 0;
}

char *WriteJsonString(char *p, const char *text) {
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

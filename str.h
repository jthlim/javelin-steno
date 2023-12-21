//---------------------------------------------------------------------------

#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

//---------------------------------------------------------------------------

class Str {
public:
  static bool IsFingerSpellingCommand(const char *p);
  static bool IsJoinPrevious(const char *p);
  static bool ContainsKeyCode(const char *p);

  static char *Join(const char *p, ...);

  // Returns an allocated string.
  static char *Asprintf(const char *p, ...);
  static size_t Sprintf(char *target, const char *p, ...);

  // Threadsafe version of strndup.
  static char *DupN(const char *p, size_t length);

  // Threadsafe version of strdup.
  static char *Dup(const char *p);

  static inline bool Eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
  }

  static inline bool ShortEq(const char *a, const char *b) {
    while (*a == *b) {
      if (!*a) {
        return true;
      }
      ++a;
      ++b;
    }
    return false;
  }

  static bool HasPrefix(const char *p, const char *prefix);

  // Returns the end of the write area. buffer must have enough space to store
  // the result;
  static char *WriteJson(char *buffer, const char *text);

  // Needs to be invoked with Str::Length<>(...), otherwise the strlen()
  // version is used instead.
  template <size_t N> static size_t Length(const char (&text)[N]) {
    return N - 1;
  }
  static size_t Length(const uint8_t *p) { return strlen((const char *)p); }
  static size_t Length(const char *p) { return strlen(p); }

  // p must start with a '-' or digit, and stops parsing at the first non-digit.
  // Returns the character after the number if successful, null otherwise.
  static const char *ParseInteger(int *result, const char *p,
                                  bool allowNegative = true);
};

//---------------------------------------------------------------------------

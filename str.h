//---------------------------------------------------------------------------

#pragma once
#include <string.h>

//---------------------------------------------------------------------------

class Str {
public:
  static bool IsFingerSpellingCommand(const char *p);
  static bool IsJoinNext(const char *p);
  static bool IsJoinPrevious(const char *p);
  static bool ContainsKeyCode(const char *p);

  static char *Join(const char *p, ...);

  // Returns an allocated string.
  static char *Asprintf(const char *p, ...) __printflike(1, 2);

  // Threadsafe version of strndup.
  static char *DupN(const char *p, size_t length);

  // Threadsafe version of strdup.
  static char *Dup(const char *p);

  static inline bool Eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
  }
  static bool HasPrefix(const char *p, const char *prefix);

  // Returns the end of the write area. p must have enough space to store
  // the result;
  static char *WriteJson(char *p, const char *text);
};

//---------------------------------------------------------------------------

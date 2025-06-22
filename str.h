//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <string.h>

//---------------------------------------------------------------------------

class Str {
public:
  static bool IsFingerSpellingCommand(const char *p);
  static bool IsJoinPrevious(const char *p);
  static bool ContainsKeyCode(const char *p);
  static bool Contains(const char *p, char c) {
    return strchr(p, c) != nullptr;
  }
  static bool IsSpace(const char *p);

  static char *Join(const char *const *p, size_t n);

  template <typename... T> static inline char *Join(T... p) {
    const char *const data[] = {p...};
    return Join(data, sizeof...(p));
  }

  // Returns an allocated string.
  static char *Asprintf(const char *p, ...);
  static size_t Sprintf(char *target, const char *p, ...);

  // Threadsafe version of strndup.
  static char *DupN(const char *p, size_t length);
  static char *DupN(const uint8_t *p, size_t length) {
    return DupN((const char *)p, length);
  }

  // Threadsafe version of strdup.
  static char *Dup(const char *p);
  static char *CreateEmpty();

  static inline bool Eq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
  }
  static bool Eq(const char *a, const char *b, size_t bLength);

  // Compares Trim(a) == b without allocations.
  static bool TrimEq(const char *a, const char *b);

  static inline int Compare(const char *a, const char *b) {
    return strcmp(a, b);
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
  static bool HasPrefix(const char *p, const char *prefix, size_t prefixLength);
  static bool HasSuffix(const char *p, const char *suffix);

  // Returns the end of the write area. buffer must have enough space to store
  // the result;
  static char *WriteJson(char *buffer, const char *text);

  // Removes leading and trailing whitespace.
  // Always returns a newly allocated string.
  static char *Trim(const char *data);

  template <size_t N>
  static consteval size_t ConstLength(const char (&text)[N]) {
    return N - 1;
  }
  static size_t Length(const uint8_t *p) { return strlen((const char *)p); }
  static size_t Length(const char *p) { return strlen(p); }

  // p must start with a '-' or digit, and stops parsing at the first non-digit.
  // Returns the character after the number if successful, null otherwise.
  static const char *ParseInteger(int *result, const char *p,
                                  bool allowNegative = true);

  // Returns nullptr if there's no word character.
  // p can be null.
  static const char *AdvanceToWordCharacter(const char *p);

  // Returns end of string if there's no following non-word character.
  // p cannot be null.
  static const char *AdvanceToNonWordCharacter(const char *p);
};

//---------------------------------------------------------------------------

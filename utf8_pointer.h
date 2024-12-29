//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Utf8Pointer {
public:
  Utf8Pointer() = default;
  Utf8Pointer(const char *p) : p((uint8_t *)p) {}
  Utf8Pointer(const uint8_t *p) : p((uint8_t *)p) {}

  uint32_t operator*() const {
    if (*p < 0x80) {
      return *p;
    }
    return Read();
  }
  void operator++() { p += DECODE_TABLE[*p]; }
  Utf8Pointer operator++(int) {
    Utf8Pointer snapshot(p);
    operator++();
    return snapshot;
  }

  void SetAndAdvance(uint32_t c);
  void SetAsciiAndAdvance(uint8_t c) { *p++ = c; }
  void SetTerminatingNull() { *p = '\0'; }
  static uint32_t BytesForCharacterCode(uint32_t c);

  bool operator<(const char *other) const { return p < (const uint8_t *)other; }

  char *GetRawPointer() const { return (char *)p; }

  bool IsEndOfString() const { return *p == '\0'; }

private:
  uint8_t *p;

  static const uint8_t DECODE_TABLE[];

  uint32_t Read() const;
};

//---------------------------------------------------------------------------

class Utf8 {
public:
  static size_t Length(const char *p);
};

//---------------------------------------------------------------------------
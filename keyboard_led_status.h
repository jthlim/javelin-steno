//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class KeyboardLedStatus {
public:
  KeyboardLedStatus() = default;
  KeyboardLedStatus(uint8_t value) : value(value) {}

  void Reset() { value = 0; }

  bool IsNumLockOn() const { return numLock; }
  bool IsCapsLockOn() const { return capsLock; }
  bool IsScrollLockOn() const { return scrollLock; }
  bool IsComposeOn() const { return compose; }
  bool IsKanaOn() const { return kana; }

  bool GetLedStatus(int index) const { return ((value >> index) & 1) != 0; }

  uint32_t GetValue() const { return value; }

  bool operator==(const KeyboardLedStatus &other) const {
    return value == other.value;
  }

private:
  union {
    struct {
      uint8_t numLock : 1;
      uint8_t capsLock : 1;
      uint8_t scrollLock : 1;
      uint8_t compose : 1;
      uint8_t kana : 1;
    };
    uint8_t value;
  };
};

static_assert(sizeof(KeyboardLedStatus) == 1,
              "Unexpected KeyboardLedStatus size");

//---------------------------------------------------------------------------

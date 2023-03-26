//---------------------------------------------------------------------------

#pragma once
#include "key_code.h"
#include "keyboard_layout.h"

#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

struct KeyboardLedStatus {
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

class Key {
public:
  static void PressRaw(KeyCode key);
  static void ReleaseRaw(KeyCode key);

  static void Press(KeyCode key);
  static void Release(KeyCode key);
  static void Flush();

  static bool IsNumLockOn();
  static bool IsCapsLockOn() { return ledStatus.capsLock; }
  static bool IsScrollLockOn() { return ledStatus.scrollLock; }
  static bool IsComposeOn() { return ledStatus.compose; }
  static bool IsKanaOn() { return ledStatus.kana; }

  static bool GetLedStatus(int index) {
    return ((ledStatus.value >> index) & 1) != 0;
  }

  static void SetKeyboardLedStatus(KeyboardLedStatus value) {
    ledStatus = value;
  }

  static void EnableHistory() { historyEnabled = true; }
  static void DisableHistory() { historyEnabled = false; }

  static bool historyEnabled;

#if RUN_TESTS
  struct HistoryEntry {
    HistoryEntry() = default;
    HistoryEntry(KeyCode code, bool isPress) : code(code), isPress(isPress) {}

    KeyCode code;
    bool isPress;
  };

  static std::vector<HistoryEntry> history;
#endif

private:
  static KeyboardLedStatus ledStatus;

  static KeyCode TranslateKey(KeyCode key);
};

//---------------------------------------------------------------------------

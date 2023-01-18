//---------------------------------------------------------------------------

#pragma once
#include "key_code.h"
#include "keyboard_layout.h"

#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class Key {
public:
  static void PressRaw(uint8_t key);
  static void ReleaseRaw(uint8_t key);

  static void Press(uint8_t key);
  static void Release(uint8_t key);
  static void Flush();

  static bool IsNumLockOn();
  static void SetIsNumLockOn(bool value);

  static void EnableHistory() { historyEnabled = true; }
  static void DisableHistory() { historyEnabled = false; }

  static bool historyEnabled;

#if RUN_TESTS
  struct HistoryEntry {
    HistoryEntry() = default;
    HistoryEntry(uint8_t code, bool isPress)
        : code((KeyCode::Value)code), isPress(isPress) {}

    KeyCode::Value code;
    bool isPress;
  };

  static std::vector<HistoryEntry> history;
#endif

private:
  static bool isNumLockOn;

  static uint8_t TranslateKey(uint8_t key);
};

//---------------------------------------------------------------------------

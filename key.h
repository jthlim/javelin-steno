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
  static void PressRaw(KeyCode key);
  static void ReleaseRaw(KeyCode key);

  static void Press(KeyCode key);
  static void Release(KeyCode key);
  static void Flush();

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
  static KeyCode TranslateKey(KeyCode key);
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once
#include "key_code.h"

#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

class Key {
public:
  static void Press(KeyCode key);
  static void Release(KeyCode key);
  static void Tap(KeyCode key) {
    Press(key);
    Release(key);
  }
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
};

//---------------------------------------------------------------------------

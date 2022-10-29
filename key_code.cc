#include "key_code.h"

#if RUN_TESTS

bool Key::historyEnabled = true;
std::vector<Key::HistoryEntry> Key::history;

void Key::Press(uint8_t key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, true));
}

void Key::Release(uint8_t key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, false));
}

bool Key::IsNumLockOn() { return false; }

#endif // RUN_TESTS

//---------------------------------------------------------------------------

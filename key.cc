//---------------------------------------------------------------------------

#include "key.h"
#include "str.h"

//---------------------------------------------------------------------------

#if RUN_TESTS

bool Key::historyEnabled = true;
std::vector<Key::HistoryEntry> Key::history;

void Key::Press(KeyCode key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, true));
}

void Key::Release(KeyCode key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, false));
}

#endif // RUN_TESTS

//---------------------------------------------------------------------------

__attribute__((weak)) void Key::Flush() {}

//---------------------------------------------------------------------------

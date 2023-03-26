//---------------------------------------------------------------------------

#include "key.h"
#include "str.h"

//---------------------------------------------------------------------------

KeyboardLedStatus Key::ledStatus;

//---------------------------------------------------------------------------

#if RUN_TESTS

bool Key::historyEnabled = true;
std::vector<Key::HistoryEntry> Key::history;

void Key::PressRaw(KeyCode key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, true));
}

void Key::ReleaseRaw(KeyCode key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, false));
}

#endif // RUN_TESTS

//---------------------------------------------------------------------------

void Key::Press(KeyCode key) { PressRaw(TranslateKey(key)); }

void Key::Release(KeyCode key) { ReleaseRaw(TranslateKey(key)); }

__attribute__((weak)) void Key::Flush() {}

__attribute__((weak)) bool Key::IsNumLockOn() { return ledStatus.numLock; }

KeyCode Key::TranslateKey(KeyCode key) {
  if (key.value >= 64) {
    return key;
  }
  const KeyboardLayoutTable *layoutTable =
      KeyboardLayout::GetActiveLayoutTable();
  return layoutTable == nullptr
             ? key
             : KeyCode::Value(layoutTable->values[key.value]);
}

//---------------------------------------------------------------------------

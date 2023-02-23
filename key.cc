//---------------------------------------------------------------------------

#include "key.h"
#include "str.h"

//---------------------------------------------------------------------------

KeyboardLedStatus Key::ledStatus;

//---------------------------------------------------------------------------

#if RUN_TESTS

bool Key::historyEnabled = true;
std::vector<Key::HistoryEntry> Key::history;

void Key::PressRaw(uint8_t key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, true));
}

void Key::ReleaseRaw(uint8_t key) {
  if (!historyEnabled) {
    return;
  }
  history.push_back(HistoryEntry(key, false));
}

#endif // RUN_TESTS

//---------------------------------------------------------------------------

void Key::Press(uint8_t key) { PressRaw(TranslateKey(key)); }

void Key::Release(uint8_t key) { ReleaseRaw(TranslateKey(key)); }

__attribute__((weak)) void Key::Flush() {}

__attribute__((weak))   bool Key::IsNumLockOn() { return ledStatus.numLock; }


uint8_t Key::TranslateKey(uint8_t key) {
  if (key >= 64) {
    return key;
  }
  const KeyboardLayoutTable *layoutTable =
      KeyboardLayout::GetActiveLayoutTable();
  return layoutTable == nullptr ? key : layoutTable->values[key];
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "key.h"
#include "str.h"

//---------------------------------------------------------------------------

bool Key::isNumLockOn = false;

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

uint8_t Key::TranslateKey(uint8_t key) {
  const KeyboardLayoutTable *layoutTable =
      KeyboardLayout::GetActiveLayoutTable();
  return layoutTable == nullptr ? key : layoutTable->values[key];
}

//---------------------------------------------------------------------------

__attribute__((weak)) bool Key::IsNumLockOn() { return isNumLockOn; }

__attribute__((weak)) void Key::SetIsNumLockOn(bool value) {
  isNumLockOn = value;
}

//---------------------------------------------------------------------------

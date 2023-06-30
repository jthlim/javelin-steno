//---------------------------------------------------------------------------

#include "keyboard_led_status.h"
#include "button_manager.h"

//---------------------------------------------------------------------------

void KeyboardLedStatus::Set(KeyboardLedStatusValue value) {
#if JAVELIN_SPLIT
  instance.Set(value);
#else
  instance = value;
#endif
  ButtonManager::ExecuteScript(ScriptId::KEYBOARD_LED_STATUS_UPDATE);
}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

KeyboardLedStatus::KeyboardLedStatusData KeyboardLedStatus::instance;

void KeyboardLedStatus::KeyboardLedStatusData::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;
  buffer.Add(SplitHandlerId::KEYBOARD_LED_STATUS, &value, 1);
}

void KeyboardLedStatus::KeyboardLedStatusData::OnDataReceived(const void *data,
                                                              size_t length) {
  instance.value = *(KeyboardLedStatusValue *)data;
}

//---------------------------------------------------------------------------

#else // JAVELIN_SPLIT

//---------------------------------------------------------------------------

KeyboardLedStatusValue KeyboardLedStatus::instance;

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

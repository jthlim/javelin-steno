//---------------------------------------------------------------------------

#include "usb_status.h"
#include "../button_manager.h"

//---------------------------------------------------------------------------

UsbStatus UsbStatus::instance;

//---------------------------------------------------------------------------

void UsbStatus::SetKeyboardLedStatus(KeyboardLedStatus status) {
  ledStatus = status;
  ButtonManager::ExecuteScript(ScriptId::KEYBOARD_LED_STATUS_UPDATE);
}

//---------------------------------------------------------------------------

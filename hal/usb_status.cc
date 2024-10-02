//---------------------------------------------------------------------------

#include "usb_status.h"
#include "../script_manager.h"

//---------------------------------------------------------------------------

UsbStatus UsbStatus::instance;

//---------------------------------------------------------------------------

void UsbStatus::SetKeyboardLedStatus(KeyboardLedStatus status) {
  ledStatus = status;
  ScriptManager::ExecuteScript(ButtonScriptId::KEYBOARD_LED_STATUS_UPDATE);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "usb_status.h"
#include "../button_script_manager.h"

//---------------------------------------------------------------------------

UsbStatus UsbStatus::instance;

//---------------------------------------------------------------------------

void UsbStatus::SetKeyboardLedStatus(KeyboardLedStatus status) {
  if (ledStatus == status) {
    return;
  }

  ledStatus = status;
  ButtonScriptManager::ExecuteScript(
      ButtonScriptId::KEYBOARD_LED_STATUS_UPDATE);
}

//---------------------------------------------------------------------------

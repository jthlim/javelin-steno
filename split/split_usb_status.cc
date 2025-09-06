//---------------------------------------------------------------------------

#include "split_usb_status.h"
#include "../button_script_manager.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitUsbStatus SplitUsbStatus::instance;

//---------------------------------------------------------------------------

void SplitUsbStatus::OnMount() {
  dirty = true;
  GetLocalUsbStatus().OnMount();
}

void SplitUsbStatus::OnUnmount() {
  dirty = true;
  GetLocalUsbStatus().OnUnmount();
}

void SplitUsbStatus::OnSuspend() {
  dirty = true;
  GetLocalUsbStatus().OnSuspend();
}

void SplitUsbStatus::OnResume() {
  dirty = true;
  GetLocalUsbStatus().OnResume();
}

void SplitUsbStatus::SetPowered(bool value) {
  dirty = true;
  GetLocalUsbStatus().SetPowered(value);
}

void SplitUsbStatus::SetKeyboardLedStatus(KeyboardLedStatus status) {
  dirty = true;
  GetLocalUsbStatus().SetKeyboardLedStatus(status);
}

void SplitUsbStatus::SetBatteryPercentage(int percentage) {
  UsbStatus &instance = GetLocalUsbStatus();
  if (percentage != instance.GetBatteryPercentage()) {
    dirty = true;
    instance.SetBatteryPercentage(percentage);
  }
}

void SplitUsbStatus::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) [[likely]] {
    return;
  }
  dirty = false;

  buffer.Add(SplitHandlerId::USB_STATUS, &GetLocalUsbStatus(),
             sizeof(UsbStatus));
}

void SplitUsbStatus::OnDataReceived(const void *data, size_t length) {
  assert(length == sizeof(UsbStatus));
  UsbStatus &instance = GetRemoteUsbStatus();
  const UsbStatus oldStatus = instance;
  memcpy(&instance, data, sizeof(UsbStatus));
  if (instance.IsConnected() != oldStatus.IsConnected() ||
      instance.IsSleeping() != oldStatus.IsSleeping()) {
    ButtonScriptManager::ExecuteScript(ButtonScriptId::CONNECTION_UPDATE);
  }
  if (instance.IsPowered() != oldStatus.IsPowered()) {
    ButtonScriptManager::ExecuteScript(ButtonScriptId::BATTERY_UPDATE);
  }
  if (instance.GetKeyboardLedStatus() != oldStatus.GetKeyboardLedStatus()) {
    ButtonScriptManager::ExecuteScript(
        ButtonScriptId::KEYBOARD_LED_STATUS_UPDATE);
  }
  if (instance.GetBatteryPercentage() != oldStatus.GetBatteryPercentage()) {
    OnReceivedBatteryPercentUpdated();
  }
}

void SplitUsbStatus::OnConnectionReset() {
  dirty = true;
  UsbStatus &status = GetRemoteUsbStatus();
  if (status.HasData()) {
    status.Reset();
    ButtonScriptManager::ExecuteScript(ButtonScriptId::CONNECTION_UPDATE);
  }
}

[[gnu::weak]] void SplitUsbStatus::OnReceivedBatteryPercentUpdated() {}

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

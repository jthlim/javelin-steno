//---------------------------------------------------------------------------

#include "split_usb_status.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitUsbStatus SplitUsbStatus::instance;

//---------------------------------------------------------------------------

void SplitUsbStatus::OnMount() {
  dirty = true;
  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  instance.OnMount();
}
void SplitUsbStatus::OnUnmount() {
  dirty = true;
  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  instance.OnUnmount();
}
void SplitUsbStatus::OnSuspend() {
  dirty = true;
  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  instance.OnSuspend();
}
void SplitUsbStatus::OnResume() {
  dirty = true;
  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  instance.OnResume();
}

void SplitUsbStatus::SetPowered(bool value) {
  dirty = true;
  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  instance.SetPowered(value);
}

void SplitUsbStatus::SetBatteryPercentage(int percentage) {
  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  if (percentage != instance.GetBatteryPercentage()) {
    dirty = true;
    instance.SetBatteryPercentage(percentage);
  }
}

void SplitUsbStatus::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;

  UsbStatus &instance = Split::IsMaster() ? UsbStatus::instance : status;
  buffer.Add(SplitHandlerId::USB_STATUS, &instance, sizeof(UsbStatus));
}

void SplitUsbStatus::OnDataReceived(const void *data, size_t length) {
  assert(length == sizeof(UsbStatus));
  UsbStatus &instance = Split::IsMaster() ? status : UsbStatus::instance;
  memcpy(&instance, data, sizeof(status));
}

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "split_usb_status.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

SplitUsbStatus SplitUsbStatus::instance;

void SplitUsbStatus::OnMount() {
  dirty = true;
  if (Split::IsMaster()) {
    UsbStatus::instance.OnMount();
  } else {
    status.OnMount();
  }
}
void SplitUsbStatus::OnUnmount() {
  dirty = true;
  if (Split::IsMaster()) {
    UsbStatus::instance.OnUnmount();
  } else {
    status.OnUnmount();
  }
}
void SplitUsbStatus::OnSuspend() {
  dirty = true;
  if (Split::IsMaster()) {
    UsbStatus::instance.OnSuspend();
  } else {
    status.OnSuspend();
  }
}
void SplitUsbStatus::OnResume() {
  dirty = true;
  if (Split::IsMaster()) {
    UsbStatus::instance.OnResume();
  } else {
    status.OnResume();
  }
}

void SplitUsbStatus::SetPowered(bool value) {
  dirty = true;
  if (Split::IsMaster()) {
    UsbStatus::instance.SetPowered(value);
  } else {
    status.SetPowered(value);
  }
}

void SplitUsbStatus::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;

  if (Split::IsMaster()) {
    buffer.Add(SplitHandlerId::USB_STATUS, &UsbStatus::instance,
               sizeof(UsbStatus::instance));

  } else {
    buffer.Add(SplitHandlerId::USB_STATUS, &status, sizeof(status));
  }
}

void SplitUsbStatus::OnDataReceived(const void *data, size_t length) {
  assert(length == sizeof(UsbStatus));
  if (Split::IsMaster()) {
    memcpy(&status, data, sizeof(status));
  } else {
    memcpy(&UsbStatus::instance, data, sizeof(UsbStatus::instance));
  }
}

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

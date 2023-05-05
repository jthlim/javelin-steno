//---------------------------------------------------------------------------

#include "split_usb_status.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

SplitUsbStatus SplitUsbStatus::instance;

void SplitUsbStatus::OnMount() {
  if (Split::IsMaster()) {
    UsbStatus::instance.OnMount();
  } else {
    dirty = true;
    status.OnMount();
  }
}
void SplitUsbStatus::OnUnmount() {
  if (Split::IsMaster()) {
    UsbStatus::instance.OnUnmount();
  } else {
    dirty = true;
    status.OnUnmount();
  }
}
void SplitUsbStatus::OnSuspend() {
  if (Split::IsMaster()) {
    UsbStatus::instance.OnSuspend();
  } else {
    dirty = true;
    status.OnSuspend();
  }
}
void SplitUsbStatus::OnResume() {
  if (Split::IsMaster()) {
    UsbStatus::instance.OnResume();
  } else {
    dirty = true;
    status.OnResume();
  }
}

void SplitUsbStatus::UpdateBuffer(TxBuffer &buffer) {
  if (dirty) {
    dirty = false;
    buffer.Add(SplitHandlerId::USB_STATUS, &status, sizeof(status));
  }
}

void SplitUsbStatus::OnDataReceived(const void *data, size_t length) {
  assert(length == sizeof(status));
  memcpy(&status, data, sizeof(status));
}

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

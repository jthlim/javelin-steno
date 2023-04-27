//---------------------------------------------------------------------------

#include "split_usb_status.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

SplitUsbStatus SplitUsbStatus::instance;

void SplitUsbStatus::OnMount() {
  status.OnMount();
  UpdateUsbStatusInstance();
}
void SplitUsbStatus::OnUnmount() {
  status.OnUnmount();
  UpdateUsbStatusInstance();
}
void SplitUsbStatus::OnSuspend() {
  status.OnSuspend();
  UpdateUsbStatusInstance();
}
void SplitUsbStatus::OnResume() {
  status.OnResume();
  UpdateUsbStatusInstance();
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
  UsbStatus::instance = status;
}

void SplitUsbStatus::UpdateUsbStatusInstance() {
  dirty = true;
  if (Split::IsMaster()) {
    UsbStatus::instance = status;
  }
}

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

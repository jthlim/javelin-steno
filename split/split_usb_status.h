//---------------------------------------------------------------------------

#pragma once
#include "../hal/usb_status.h"
#include "../split/split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

class SplitUsbStatus final : public SplitTxHandler, public SplitRxHandler {
public:
  bool IsConnected() const { return status.IsConnected(); }
  bool IsSleeping() const { return status.IsSleeping(); }

  void OnMount();
  void OnUnmount();
  void OnSuspend();
  void OnResume();

  static void RegisterTxHandler() { Split::RegisterTxHandler(&instance); }

  static void RegisterRxHandler() {
    Split::RegisterRxHandler(SplitHandlerId::USB_STATUS, &instance);
  }

  static SplitUsbStatus instance;

private:
  bool dirty;
  UsbStatus status;

  virtual void UpdateBuffer(TxBuffer &buffer);
  virtual void OnTransmitConnectionReset() { dirty = true; }
  virtual void OnDataReceived(const void *data, size_t length);
};

#else

class SplitUsbStatus : public UsbStatus {
public:
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

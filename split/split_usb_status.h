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
  bool IsPowered() const { return status.IsPowered(); }

  void OnMount();
  void OnUnmount();
  void OnSuspend();
  void OnResume();

  void SetPowered(bool value);
  void SetBatteryPercentage(int percentage);
  int GetBatteryPercentage() const { return status.GetBatteryPercentage(); }

  static void RegisterHandlers() {
    Split::RegisterTxHandler(&instance);
    Split::RegisterRxHandler(SplitHandlerId::USB_STATUS, &instance);
  }

  static SplitUsbStatus instance;

private:
  bool dirty;
  UsbStatus status;

  virtual void UpdateBuffer(TxBuffer &buffer);
  virtual void OnTransmitConnectionReset() { OnConnectionReset(); }
  virtual void OnReceiveConnectionReset() { OnConnectionReset(); }
  virtual void OnDataReceived(const void *data, size_t length);

  void OnConnectionReset();
};

#else

class SplitUsbStatus : public UsbStatus {
public:
  static void RegisterHandlers() {}
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

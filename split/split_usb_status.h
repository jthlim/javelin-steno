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

  KeyboardLedStatus GetKeyboardLedStatus() const {
    return status.GetKeyboardLedStatus();
  }
  void SetKeyboardLedStatus(KeyboardLedStatus status);

  void SetPowered(bool value);
  void SetBatteryPercentage(int percentage);
  int GetBatteryPercentage() const { return status.GetBatteryPercentage(); }

  static void RegisterHandlers() {
    Split::RegisterTxHandler(&instance);
    Split::RegisterRxHandler(SplitHandlerId::USB_STATUS, &instance);
  }

  static UsbStatus &GetLocalUsbStatus() {
    return Split::IsMaster() ? UsbStatus::instance : instance.status;
  }

  static UsbStatus &GetRemoteUsbStatus() {
    return Split::IsMaster() ? instance.status : UsbStatus::instance;
  }

  static SplitUsbStatus instance;

private:
  bool dirty;
  UsbStatus status;

  virtual void UpdateBuffer(TxBuffer &buffer) override;
  virtual void OnTransmitConnectionReset() override { OnConnectionReset(); }
  virtual void OnReceiveConnectionReset() override { OnConnectionReset(); }
  virtual void OnTransmitConnected() override { dirty = true; }
  virtual void OnDataReceived(const void *data, size_t length) override;

  void OnConnectionReset();

  void OnReceivedBatteryPercentUpdated();
};

#else

class SplitUsbStatus : public UsbStatus {
public:
  static void RegisterHandlers() {}

  void OnReceivedBatteryPercentUpdated();
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

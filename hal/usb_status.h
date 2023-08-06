//---------------------------------------------------------------------------

#pragma once
#include "../keyboard_led_status.h"
#include <stdint.h>

//---------------------------------------------------------------------------

class UsbStatus {
public:
  bool IsConnected() const { return isMounted; }
  bool IsSleeping() const { return IsConnected() && isSuspended; }
  bool IsPowered() const { return isPowered; }

  KeyboardLedStatus GetKeyboardLedStatus() const { return ledStatus; }
  void SetKeyboardLedStatus(KeyboardLedStatus status);

  int GetBatteryPercentage() const { return batteryPercentage; }

  void SetPowered(bool value) { isPowered = value; }
  void SetBatteryPercentage(int percentage) { batteryPercentage = percentage; }

  void OnMount() {
    isMounted = true;
    isSuspended = false;
  }
  void OnUnmount() { isMounted = false; }

  void OnSuspend() { isSuspended = true; }
  void OnResume() { isSuspended = false; }

  static UsbStatus instance;

private:
  bool isMounted : 1;
  bool isSuspended : 1;
  bool isPowered : 1;
  KeyboardLedStatus ledStatus;
  uint8_t batteryPercentage;
};

static_assert(sizeof(UsbStatus) <= 4, "Unexpected UsbStatus size");

//---------------------------------------------------------------------------

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

  bool HasData() {
    return flags != 0 || ledStatus.GetValue() != 0 || batteryPercentage != 0;
  }
  void Reset() {
    flags = 0;
    ledStatus.Reset();
    batteryPercentage = 0;
  }

  static UsbStatus instance;

private:
  union {
    struct {
      bool isMounted : 1;
      bool isSuspended : 1;
      bool isPowered : 1;
    };
    uint8_t flags;
  };
  KeyboardLedStatus ledStatus;
  uint8_t batteryPercentage;
};

static_assert(sizeof(UsbStatus) <= 4, "Unexpected UsbStatus size");

//---------------------------------------------------------------------------

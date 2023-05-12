//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class UsbStatus {
public:
  bool IsConnected() const { return isMounted; }
  bool IsSleeping() const { return IsConnected() && isSuspended; }
  bool IsPowered() const { return isPowered; }

  uint32_t GetMountCount() const { return mountCount; }
  uint32_t GetSuspendCount() const { return suspendCount; }

  void SetPowered(bool value) { isPowered = value; }

  void OnMount() {
    isMounted = true;
    isSuspended = false;
    mountCount++;
  }
  void OnUnmount() { isMounted = false; }

  void OnSuspend() {
    isSuspended = true;
    suspendCount++;
  }
  void OnResume() { isSuspended = false; }

  static UsbStatus instance;

private:
  bool isMounted;
  bool isSuspended;
  bool isPowered;
  uint32_t mountCount;
  uint32_t suspendCount;
};

//---------------------------------------------------------------------------

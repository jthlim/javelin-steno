//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class UsbStatus {
public:
  static bool IsMounted() { return instance.isMounted; }
  static bool IsSuspended() { return instance.isSuspended; }
  static uint32_t GetMountCount() { return instance.mountCount; }
  static uint32_t GetSuspendCount() { return instance.suspendCount; }

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
  uint32_t mountCount;
  uint32_t suspendCount;
};

//---------------------------------------------------------------------------

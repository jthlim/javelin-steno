//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

//---------------------------------------------------------------------------

class UsbStatus {
public:
  static bool IsMounted() { return instance.isMounted; }
  static bool IsSuspended() { return instance.isSuspended; }
  static size_t GetMountCount() { return instance.mountCount; }
  static size_t GetSuspendCount() { return instance.suspendCount; }

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
  size_t mountCount;
  size_t suspendCount;
};

//---------------------------------------------------------------------------

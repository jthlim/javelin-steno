//---------------------------------------------------------------------------

#pragma once
#include "split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
class Bootrom final : public SplitTxHandler, SplitRxHandler {
#else
class Bootrom {
#endif
public:
  static void Launch();

#if JAVELIN_SPLIT
  static void LaunchSlave() { instance.launchSlave = true; }

  static void RegisterTxHandler() { Split::RegisterTxHandler(&instance); }

  static void RegisterRxHandler() {
    Split::RegisterRxHandler(SplitHandlerId::BOOTROM, &instance);
  }

  virtual void UpdateBuffer(TxBuffer &buffer);
  virtual void OnDataReceived(const void *data, size_t length);

private:
  bool launchSlave;

  static Bootrom instance;

#else
  static void LaunchSlave() {}

  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
#endif
};

//---------------------------------------------------------------------------

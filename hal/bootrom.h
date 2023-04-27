//---------------------------------------------------------------------------

#pragma once
#include "../split/split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
class Bootrom final : public SplitTxHandler, SplitRxHandler {
#else
class Bootrom {
#endif
public:
  static void Launch();
  static void LaunchBootrom(void *const, const char *commandLine) { Launch(); }

#if JAVELIN_SPLIT
  static void LaunchSlave() { instance.launchSlave = true; }
  static void LaunchSlaveBootrom(void *const, const char *commandLine) {
    LaunchSlave();
  }

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
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
#endif
};

//---------------------------------------------------------------------------

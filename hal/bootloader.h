//---------------------------------------------------------------------------

#pragma once
#include "../split/split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
class Bootloader final : public SplitTxHandler, SplitRxHandler {
#else
class Bootloader {
#endif
public:
  static void Launch();
  static void LaunchBootloader(void *const, const char *commandLine) {
    Launch();
  }

#if JAVELIN_SPLIT
  static void LaunchSlave() { instance.launchSlave = true; }
  static void LaunchSlaveBootloader(void *const, const char *commandLine) {
    LaunchSlave();
  }

  static void RegisterTxHandler() { Split::RegisterTxHandler(&instance); }

  static void RegisterRxHandler() {
    Split::RegisterRxHandler(SplitHandlerId::BOOTLOADER, &instance);
  }

  virtual void UpdateBuffer(TxBuffer &buffer);
  virtual void OnDataReceived(const void *data, size_t length);

private:
  bool launchSlave;

  static Bootloader instance;

#else
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
#endif
};

//---------------------------------------------------------------------------

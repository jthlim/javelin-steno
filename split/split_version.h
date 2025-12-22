//---------------------------------------------------------------------------

#pragma once
#include "split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

#if !defined(JAVELIN_SPLIT_VERSION_INCREMENT)
#define JAVELIN_SPLIT_VERSION_INCREMENT 0
#endif

class SplitVersion :
#if JAVELIN_SPLIT_IS_MASTER
    public SplitRxHandler
#else
    public SplitTxHandler
#endif
{
public:
  static void RegisterTxHandler() {
#if !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if JAVELIN_SPLIT_IS_MASTER
    void OnReceiveConnectionReset();
    Split::RegisterRxHandler(SplitHandlerId::VERSION, &instance);
#endif
  }

private:
  static constexpr uint32_t VERSION = 5 + JAVELIN_SPLIT_VERSION_INCREMENT;

#if JAVELIN_SPLIT_IS_MASTER
  void OnReceiveConnectionReset() final;
  void OnDataReceived(const void *data, size_t length) final;

  void ShowError();
  void ClearError();
#else
  bool isDirty;

  void OnTransmitConnected() final { isDirty = true; }
  void UpdateBuffer(TxBuffer &buffer) final;
#endif

  static SplitVersion instance;
};

#else

class SplitVersion {
public:
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

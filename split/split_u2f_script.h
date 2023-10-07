//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

#if JAVELIN_U2F
#include "split.h"

class SplitU2fScript : public SplitTxHandler, public SplitRxHandler {
public:
  static void SetWaitingForUserPresence(bool value) {
    instance.dirty = true;
    instance.data.isWaitingForUserPresence = value;
  }

  static bool IsWaitingForUserPresence() {
    return instance.data.isWaitingForUserPresence;
  }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if JAVELIN_SPLIT
    Split::RegisterRxHandler(SplitHandlerId::U2F_SCRIPT, &instance);
#endif
  }

private:
  bool dirty;
  struct Data {
    bool isWaitingForUserPresence;

    bool operator==(const Data &other) {
      return isWaitingForUserPresence == other.isWaitingForUserPresence;
    }
  };
  Data data;

  virtual void OnTransmitConnectionReset() override { dirty = true; }
  virtual void OnReceiveConnectionReset() override;
  virtual void UpdateBuffer(TxBuffer &buffer) override;
  virtual void OnDataReceived(const void *data, size_t length) override;

  static SplitU2fScript instance;
};

#else

class SplitU2fScript {
public:
  static void SetWaitingForUserPresence(bool value) {}
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif

//---------------------------------------------------------------------------

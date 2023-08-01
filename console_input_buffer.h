//---------------------------------------------------------------------------

#pragma once
#include "hal/connection.h"
#include "queue.h"
#include "split/split.h"

//---------------------------------------------------------------------------

// This should be rewritten to handle multiple active clients on multiple hosts
// having their data properly separated.

class ConsoleInputBuffer {
public:
  static void Add(const uint8_t *data, size_t length, ConnectionId connectionId,
                  uint16_t connectionHandle = 0) {
    instance.Add(data, length, connectionId, connectionHandle);
  };

  static void Process() { instance.Process(); }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if JAVELIN_SPLIT
    Split::RegisterRxHandler(SplitHandlerId::CONSOLE, &instance);
#endif
  }

private:
  struct EntryData {
    ConnectionId connectionId;
    uint16_t connectionHandle; // For ble connection handle.
    size_t length;
    char data[0];
  };

#if JAVELIN_SPLIT
  struct ConsoleInputBufferData : public Queue<EntryData>,
                                  public SplitTxHandler,
                                  SplitRxHandler {
#else
  struct ConsoleInputBufferData : public Queue<EntryData> {
#endif
    void Add(const uint8_t *data, size_t length, ConnectionId connectionId,
             uint16_t connectionHandle = 0);
    void Process();

    static QueueEntry<EntryData> *CreateEntry(const void *data, size_t length,
                                              ConnectionId connectionId,
                                              uint16_t connectionHandle);

#if JAVELIN_SPLIT
    // Start off connected so that any messages coming in immediately after
    // boot up are queued to send to the master.
    bool isConnected = true;

    virtual void UpdateBuffer(TxBuffer &buffer) override;
    virtual void OnDataReceived(const void *data, size_t length) override;
    virtual void OnTransmitConnectionReset() override { isConnected = false; }
    virtual void OnTransmitConnected() override { isConnected = true; }
#endif
  };

  static ConsoleInputBufferData instance;
};

//---------------------------------------------------------------------------

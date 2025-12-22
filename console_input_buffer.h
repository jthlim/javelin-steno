//---------------------------------------------------------------------------

#pragma once
#include "container/queue.h"
#include "hal/connection.h"
#include "split/split.h"

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

class ConsoleInputBuffer {
public:
  static void Add(const uint8_t *data, size_t length, ConnectionId connectionId,
                  uint16_t connectionHandle = 0) {
    instance.Add(data, length, connectionId, connectionHandle);
  };

  static void Process() { instance.Process(); }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
    Split::RegisterRxHandler(SplitHandlerId::CONSOLE, &instance);
#endif
  }

#if JAVELIN_SPLIT
  static void SetConsoleModeBinding(void *context, const char *commandLine);
  static void AddConsoleCommands(Console &console);
  static void OnConsoleModeChanged();
#endif

private:
  struct EntryData {
    uint16_t connectionHandle; // For ble connection handle.
    size_t length;

    // Data below this is sent in the TxBuffer.
    ConnectionId connectionId;
    char data[0];
  };

#if JAVELIN_SPLIT
  struct ConsoleInputBufferData final : public Queue<EntryData>,
#if JAVELIN_SPLIT_IS_MASTER
                                        public SplitRxHandler
#else
                                        public SplitTxHandler
#endif
  {
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
    bool passthroughConsoleToMaster = true;

#if JAVELIN_SPLIT_IS_MASTER
    void OnDataReceived(const void *data, size_t length) final;
#else
    void UpdateBuffer(TxBuffer &buffer) final;
    void OnTransmitConnectionReset() final { isConnected = false; }
    void OnTransmitConnected() final { isConnected = true; }
#endif
#endif
  };

  static ConsoleInputBufferData instance;
};

//---------------------------------------------------------------------------

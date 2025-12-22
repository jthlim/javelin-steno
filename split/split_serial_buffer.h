//---------------------------------------------------------------------------

#pragma once
#include "../container/cyclic_queue.h"
#include "split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

class SplitSerialBuffer {
public:
  static void Add(const void *data, size_t length) {
    instance.Add((const uint8_t *)data, length);
  }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterRxHandler(SplitHandlerId::SERIAL, &instance);
#endif
  }

private:
  struct SplitSerialBufferData :
#if JAVELIN_SPLIT_IS_MASTER
      public SplitTxHandler
#else
      public SplitRxHandler
#endif
  {
    void Add(const uint8_t *data, size_t length);

#if JAVELIN_SPLIT_IS_MASTER
    void UpdateBuffer(TxBuffer &buffer) final;
#else
    void OnDataReceived(const void *data, size_t length) final;
#endif

    static void ClearQueue();

    static const size_t MAXIMUM_SERIAL_QUEUE_SIZE = 128;
    CyclicQueue<uint8_t, MAXIMUM_SERIAL_QUEUE_SIZE> queue;
  };

  static SplitSerialBufferData instance;
};

#else

class SplitSerialBuffer {
public:
  static void Add(const void *data, size_t length) {}

  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

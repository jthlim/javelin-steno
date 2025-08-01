//---------------------------------------------------------------------------

#pragma once
#include "../container/queue.h"
#include "split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

class SplitMidi {
public:
  static void Add(const uint8_t *data, size_t length) {
    instance.Add(data, length);
  }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }

  static void RegisterRxHandler() {
#if !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterRxHandler(SplitHandlerId::MIDI, &instance);
#endif
  }

private:
  struct EntryData {
    size_t length;
    uint8_t data[0];
  };

  struct SplitMidiData : public Queue<EntryData>,
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

    static QueueEntry<EntryData> *CreateEntry(const uint8_t *data,
                                              size_t length);
  };

  static SplitMidiData instance;
};

#else

class SplitMidi {
public:
  static void Add(const uint8_t *data, size_t length) {}

  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

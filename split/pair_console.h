//---------------------------------------------------------------------------

#pragma once
#include "../queue.h"
#include "split.h"

//---------------------------------------------------------------------------

struct PairConsoleEntryData {
  size_t length;
  char data[0];
};

#if JAVELIN_SPLIT
struct PairConsole : public Queue<PairConsoleEntryData>,
                     public SplitTxHandler,
                     SplitRxHandler {
#else
struct PairConsole : public Queue<EntryData> {
#endif
public:
  static void Add(const char *data, size_t length) {
    instance.AddInternal(data, length);
  };

  static void Process() { instance.ProcessInternal(); }

  static void RegisterHandlers() {
#if JAVELIN_SPLIT
#if JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#else
    Split::RegisterRxHandler(SplitHandlerId::PAIR_CONSOLE, &instance);
#endif
#endif
  }

  static void PairBinding(void *context, const char *commandLine);

private:
  void AddInternal(const char *data, size_t length);
  void ProcessInternal();

  static QueueEntry<PairConsoleEntryData> *CreateEntry(const void *data,
                                                       size_t length);

#if JAVELIN_SPLIT
  virtual void UpdateBuffer(TxBuffer &buffer) override;
  virtual void OnDataReceived(const void *data, size_t length) override;
#endif

  static PairConsole instance;
};

//---------------------------------------------------------------------------

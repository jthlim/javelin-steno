//---------------------------------------------------------------------------

#pragma once
#include "../queue.h"
#include "split.h"

//---------------------------------------------------------------------------

class Console;

#if JAVELIN_SPLIT

struct SplitConsoleEntryData {
  size_t length;
  char data[0];
};

class SplitConsole : Queue<SplitConsoleEntryData>,
                     SplitTxHandler,
                     SplitRxHandler

{
public:
  static void Add(const char *data, size_t length) {
    instance.AddInternal(data, length);
  };

  static void Process() { instance.ProcessInternal(); }

  static void RegisterHandlers() {
#if JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#else
    Split::RegisterRxHandler(SplitHandlerId::PAIR_CONSOLE, &instance);
#endif
  }

  static void AddConsoleCommands(Console &console);
  static void PairBinding(void *context, const char *commandLine);

private:
  void AddInternal(const char *data, size_t length);
  void ProcessInternal();

  static QueueEntry<SplitConsoleEntryData> *CreateEntry(const void *data,
                                                        size_t length);

  virtual void UpdateBuffer(TxBuffer &buffer) override;
  virtual void OnDataReceived(const void *data, size_t length) override;

  static SplitConsole instance;
};

#else

class SplitConsole {
public:
  static void Process() {}
  static void RegisterHandlers() {}
  static void AddConsoleCommands(Console &console) {}
};

#endif

//---------------------------------------------------------------------------

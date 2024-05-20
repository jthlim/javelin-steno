//---------------------------------------------------------------------------

#include "pair_console.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

PairConsole PairConsole::instance;

//---------------------------------------------------------------------------

inline QueueEntry<PairConsoleEntryData> *
PairConsole::CreateEntry(const void *data, size_t length) {
  QueueEntry<PairConsoleEntryData> *entry =
      new (length) QueueEntry<PairConsoleEntryData>;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void PairConsole::AddInternal(const char *data, size_t length) {
  AddEntry(CreateEntry(data, length));
}

void PairConsole::ProcessInternal() {
#if !JAVELIN_SPLIT_IS_MASTER
  while (head) {
    Console::RunCommand(head->data.data, NullWriter::instance);
    RemoveHead();
  }
#endif
}

void PairConsole::UpdateBuffer(TxBuffer &buffer) {
#if JAVELIN_SPLIT_IS_MASTER
  while (head) {
    if (!buffer.Add(SplitHandlerId::PAIR_CONSOLE, head->data.data,
                    head->data.length)) {
      return;
    }

    RemoveHead();
  }
#endif
}

void PairConsole::OnDataReceived(const void *data, size_t length) {
  AddInternal((const char *)data, length);
}

//---------------------------------------------------------------------------

void PairConsole::PairBinding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR missing pair command\n\n");
    return;
  }

  if (!Connection::IsPairConnected()) {
    Console::Printf("ERR pair not connected\n\n");
    return;
  }

  ++p;
  size_t length = Str::Length(p) + 1;
  Add(p, length);

  Console::SendOk();
}

void PairConsole::AddConsoleCommands(Console &console) {
  console.RegisterCommand("pair", "Runs a command on the pair's console",
                          &PairConsole::PairBinding, nullptr);
}

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

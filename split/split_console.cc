//---------------------------------------------------------------------------

#include "split_console.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

SplitConsole SplitConsole::instance;

//---------------------------------------------------------------------------

inline QueueEntry<SplitConsoleEntryData> *
SplitConsole::CreateEntry(const void *data, size_t length) {
  QueueEntry<SplitConsoleEntryData> *entry =
      new (length) QueueEntry<SplitConsoleEntryData>;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void SplitConsole::AddInternal(const char *data, size_t length) {
  AddEntry(CreateEntry(data, length));
}

void SplitConsole::ProcessInternal() {
#if !JAVELIN_SPLIT_IS_MASTER
  while (head) {
    Console::RunCommand(head->data.data, NullWriter::instance);
    RemoveHead();
  }
#endif
}

void SplitConsole::UpdateBuffer(TxBuffer &buffer) {
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

void SplitConsole::OnDataReceived(const void *data, size_t length) {
  AddInternal((const char *)data, length);
}

//---------------------------------------------------------------------------

void SplitConsole::PairBinding(void *context, const char *commandLine) {
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
  const size_t length = Str::Length(p) + 1;
  Add(p, length);

  Console::SendOk();
}

void SplitConsole::AddConsoleCommands(Console &console) {
  console.RegisterCommand("pair", "Runs a command on the pair's console",
                          &SplitConsole::PairBinding, nullptr);
}

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

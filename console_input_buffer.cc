//---------------------------------------------------------------------------

#include "console_input_buffer.h"
#include "console.h"
#include "str.h"
#include <string.h>

//---------------------------------------------------------------------------

ConsoleInputBuffer::ConsoleInputBufferData ConsoleInputBuffer::instance;

//---------------------------------------------------------------------------

inline QueueEntry<ConsoleInputBuffer::EntryData> *
ConsoleInputBuffer::ConsoleInputBufferData::CreateEntry(
    const void *data, size_t length, ConnectionId connectionId,
    uint16_t connectionHandle) {
  QueueEntry<EntryData> *entry = new (length) QueueEntry<EntryData>;
  entry->data.connectionId = connectionId;
  entry->data.connectionHandle = connectionHandle;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void ConsoleInputBuffer::ConsoleInputBufferData::Add(
    const uint8_t *data, size_t length, ConnectionId connectionId,
    uint16_t connectionHandle) {
  AddEntry(CreateEntry(data, length, connectionId, connectionHandle));
}

void ConsoleInputBuffer::ConsoleInputBufferData::Process() {
  if (!head) [[likely]] {
    return;
  }

#if JAVELIN_SPLIT
  if (Split::IsSlave() && isConnected && passthroughConsoleToMaster) {
    // This will be dealt with using UpdateBuffer() instead.
    return;
  }
#endif

  while (head) {
    ConsoleWriter::SetConnection(head->data.connectionId,
                                 head->data.connectionHandle);
    Console::instance.HandleInput(head->data.data, head->data.length);
    RemoveHead();
    Console::Flush();
  }
  ConsoleWriter::SetConnection(ConnectionId::ACTIVE, 0);
}

#if JAVELIN_SPLIT
#if JAVELIN_SPLIT_IS_MASTER

void ConsoleInputBuffer::ConsoleInputBufferData::OnDataReceived(
    const void *data, size_t length) {
  Add((const uint8_t *)data, length, ConnectionId::USB_PAIR);
}

#else
void ConsoleInputBuffer::ConsoleInputBufferData::UpdateBuffer(
    TxBuffer &buffer) {
  if (!head) [[likely]] {
    return;
  }

  if (!passthroughConsoleToMaster) {
    // This will be dealt with using Process() instead.
    return;
  }

  while (head) {
    if (!buffer.Add(SplitHandlerId::CONSOLE, head->data.data,
                    head->data.length)) {
      return;
    }

    RemoveHead();
  }
}
#endif
#endif

#if JAVELIN_SPLIT
void ConsoleInputBuffer::SetConsoleModeBinding(void *context,
                                               const char *commandLine) {
  const char *mode = strchr(commandLine, ' ');
  if (!mode) {
    Console::Printf("ERR No console mode specified\n\n");
    return;
  }

  ++mode;
  if (Str::Eq(mode, "passthrough")) {
    instance.passthroughConsoleToMaster = true;
  } else if (Str::Eq(mode, "local")) {
    instance.passthroughConsoleToMaster = false;
  } else {
    Console::Printf("ERR Unable to set console mode: \"%s\"\n\n", mode);
    return;
  }

  Console::SendOk();
  Console::Flush();
  OnConsoleModeChanged();
}

void ConsoleInputBuffer::AddConsoleCommands(Console &console) {
  console.RegisterCommand(
      "set_console_mode",
      "Controls where console commands are run [\"passthrough\", \"local\"]",
      &ConsoleInputBuffer::SetConsoleModeBinding, nullptr);
}

[[gnu::weak]] void ConsoleInputBuffer::OnConsoleModeChanged() {}

#endif

//---------------------------------------------------------------------------

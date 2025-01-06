//---------------------------------------------------------------------------

#include "console_input_buffer.h"
#include "console.h"
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
  if (!head) {
    return;
  }

#if JAVELIN_SPLIT
  if (Split::IsSlave() && isConnected) {
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

//---------------------------------------------------------------------------

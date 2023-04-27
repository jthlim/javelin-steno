//---------------------------------------------------------------------------

#include "console_input_buffer.h"
#include "clock.h"
#include "console.h"
#include <string.h>

//---------------------------------------------------------------------------

ConsoleInputBuffer::ConsoleInputBufferData ConsoleInputBuffer::instance;

//---------------------------------------------------------------------------

QueueEntry<ConsoleInputBuffer::EntryData> *
ConsoleInputBuffer::ConsoleInputBufferData::CreateEntry(const void *data,
                                                        size_t length) {
  QueueEntry<EntryData> *entry = new (length) QueueEntry<EntryData>;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void ConsoleInputBuffer::ConsoleInputBufferData::Add(const uint8_t *data,
                                                     size_t length) {
  QueueEntry<EntryData> *entry = CreateEntry(data, length);
  AddEntry(entry);
}

void ConsoleInputBuffer::ConsoleInputBufferData::Process() {
#if JAVELIN_SPLIT
  if (Split::IsSlave() && isConnected) {
    // This will be dealt with using UpdateBuffer() instead.
    return;
  }
#endif

  while (head) {
    Console::instance.HandleInput(head->data.data, head->data.length);
    RemoveHead();
  }
  Console::Flush();
}

#if JAVELIN_SPLIT
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

void ConsoleInputBuffer::ConsoleInputBufferData::OnDataReceived(
    const void *data, size_t length) {
  Add((const uint8_t *)data, length);
}
#endif

//---------------------------------------------------------------------------

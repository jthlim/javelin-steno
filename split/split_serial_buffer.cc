//---------------------------------------------------------------------------

#include "split_serial_buffer.h"
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitSerialBuffer::SplitSerialBufferData SplitSerialBuffer::instance;

//---------------------------------------------------------------------------

QueueEntry<SplitSerialBuffer::EntryData> *
SplitSerialBuffer::SplitSerialBufferData::CreateEntry(const uint8_t *data,
                                                      size_t length) {
  QueueEntry<EntryData> *entry = new (length) QueueEntry<EntryData>;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void SplitSerialBuffer::SplitSerialBufferData::Add(const uint8_t *data,
                                                   size_t length) {
  QueueEntry<EntryData> *entry = CreateEntry(data, length);
  AddEntry(entry);
}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_IS_MASTER

void SplitSerialBuffer::SplitSerialBufferData::UpdateBuffer(TxBuffer &buffer) {
  if (!head) [[likely]] {
    return;
  }

  while (head) {
    if (!buffer.Add(SplitHandlerId::SERIAL, &head->data.data,
                    head->data.length)) {
      return;
    }

    RemoveHead();
  }
}

#endif

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

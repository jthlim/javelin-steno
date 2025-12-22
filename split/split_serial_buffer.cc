//---------------------------------------------------------------------------

#include "split_serial_buffer.h"
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitSerialBuffer::SplitSerialBufferData SplitSerialBuffer::instance;

//---------------------------------------------------------------------------

[[gnu::weak]]
void SplitSerialBuffer::SplitSerialBufferData::ClearQueue() {}

//---------------------------------------------------------------------------

void SplitSerialBuffer::SplitSerialBufferData::Add(const uint8_t *data,
                                                   size_t length) {
  while (length) {
    while (queue.IsFull()) {
      ClearQueue();
    }

    const size_t available = queue.GetAvailable();
    const size_t transferCount = available > length ? length : available;

    for (size_t i = 0; i < transferCount; ++i) {
      queue.Add(*data++);
    }

    length -= transferCount;
  }
}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_IS_MASTER

void SplitSerialBuffer::SplitSerialBufferData::UpdateBuffer(TxBuffer &buffer) {
  if (queue.IsEmpty()) [[likely]] {
    return;
  }

  const size_t count = queue.GetCount();
  uint8_t *target = buffer.Add(SplitHandlerId::SERIAL, count);
  if (target == nullptr) {
    return;
  }
  for (const uint8_t c : queue) {
    *target++ = c;
  }
  queue.RemoveFront(count);
}

#endif

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------

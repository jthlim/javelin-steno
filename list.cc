//---------------------------------------------------------------------------

#include "list.h"
#include <string.h>

//---------------------------------------------------------------------------

void _ListBase::Add(const void *data, size_t elementSize) {
  const size_t capacity = GetCapacity(count);
  if (count == capacity) {
    const size_t newCapacity = GetCapacity(count + 1);
    uint8_t *newBuffer = (uint8_t *)malloc(newCapacity * elementSize);
    memcpy(newBuffer, buffer, elementSize * count);

    // Do this copy before freeing to avoid problems when adding elements from
    // the list, i.e.
    // ```
    //   list.add(list[3]);
    // ```
    memcpy(newBuffer + elementSize * count, data, elementSize);
    free(buffer);

    buffer = newBuffer;
  } else {
    memcpy(buffer + elementSize * count, data, elementSize);
  }
  ++count;
}

void _ListBase::AddCount(const void *data, size_t n, size_t elementSize) {
  const size_t capacity = GetCapacity(count);
  const size_t newCount = count + n;
  if (newCount > capacity) {
    const size_t newCapacity = GetCapacity(newCount);
    uint8_t *newBuffer = (uint8_t *)malloc(newCapacity * elementSize);
    memcpy(newBuffer, buffer, elementSize * count);

    // Do this copy before freeing to avoid problems when adding elements from
    // the list, i.e.
    // ```
    //   list.add(list[3]);
    // ```
    memcpy(newBuffer + elementSize * count, data, n * elementSize);
    free(buffer);

    buffer = newBuffer;
  } else {
    memcpy(buffer + elementSize * count, data, n * elementSize);
  }
  count = newCount;
}

void _ListBase::InsertAt(const void *data, size_t index, size_t elementSize) {
  const size_t capacity = GetCapacity(count);
  if (count == capacity) {
    const size_t newCapacity = GetCapacity(count + 1);
    uint8_t *newBuffer = (uint8_t *)malloc(newCapacity * elementSize);
    memcpy(newBuffer, buffer, index * elementSize);
    memcpy(newBuffer + (index + 1) * elementSize, buffer + index * elementSize,
           (count - index) * elementSize);

    free(buffer);
    buffer = newBuffer;
  } else {
    memmove(buffer + (index + 1) * elementSize, buffer + index * elementSize,
            (count - index) * elementSize);
  }
  ++count;
}

size_t _ListBase::GetCapacity(size_t count) {
  if (count == 0) {
    return 0;
  }
  size_t powerOf2 = 4;
  while (powerOf2 < count) {
    powerOf2 <<= 1;
  }
  return powerOf2;

  // Alternative, but since most lists used are small, this isn't so
  // beneficial.

  // size_t x = (count - 1) >> 2;

  // // Round up to power of 2.
  // x |= (x >> 1);
  // x |= (x >> 2);
  // x |= (x >> 4);
  // x |= (x >> 8);
  // // Lists will never have > 256k elements.
  // // x |= (x >> 16);

  // return (x + 1) << 2;
}

//---------------------------------------------------------------------------

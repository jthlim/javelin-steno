//---------------------------------------------------------------------------

#include "list.h"
#include <string.h>

//---------------------------------------------------------------------------

void _ListBase::Add(const void *data, size_t elementSize) {
  size_t capacity = GetCapacity(count);
  if (count == capacity) {
    size_t newCapacity = GetCapacity(count + 1);
    uint8_t *newBuffer = (uint8_t *)malloc(newCapacity * elementSize);
    memcpy(newBuffer, buffer, elementSize * count);

    // Do this copy before freeing to avoid problems when adding elements from
    // the list, i.e.
    // ```
    //   list.add(list[3]);
    // ```
    memcpy(newBuffer + elementSize * count, data, elementSize);
    ++count;
    free(buffer);

    buffer = newBuffer;
  } else {
    memcpy(buffer + elementSize * count, data, elementSize);
    ++count;
  }
}

size_t _ListBase::GetCapacity(size_t count) {
  if (count == 0) {
    return 0;
  }
  size_t powerOf2 = 8;
  while (powerOf2 < count) {
    powerOf2 <<= 1;
  }
  return powerOf2;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "pool_allocate.h"
#include "assert.h"

//---------------------------------------------------------------------------

void *PoolAllocateData::Allocate(size_t blockSize, size_t size) {
  assert(size <= blockSize);

  if (sizeRemaining < size) {
    sizeRemaining = blockSize;
    data = (uint8_t *)malloc(blockSize);
  }

  sizeRemaining -= size;
  void *result = data;
  data += size;
  return result;
}

//---------------------------------------------------------------------------

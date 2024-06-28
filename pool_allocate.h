//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct PoolAllocateData {
  size_t sizeRemaining = 0;
  uint8_t *data = nullptr;

  void Reset(size_t blockSize);
  void *Allocate(size_t blockSize, size_t size);
};

//---------------------------------------------------------------------------

template <typename T, size_t BLOCK_SIZE> class PoolAllocate {
public:
  static void *operator new(size_t size) noexcept;

  static void ResetPoolAllocator() { data.Reset(BLOCK_SIZE); }

private:
  static PoolAllocateData data;
};

//---------------------------------------------------------------------------

template <typename T, size_t BLOCK_SIZE>
void *PoolAllocate<T, BLOCK_SIZE>::operator new(size_t size) noexcept {
  return data.Allocate(BLOCK_SIZE, size);
}

template <typename T, size_t BLOCK_SIZE>
PoolAllocateData PoolAllocate<T, BLOCK_SIZE>::data;

//---------------------------------------------------------------------------

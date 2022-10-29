//---------------------------------------------------------------------------

#include "flash.h"
#include "console.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

uint32_t Flash::erasedBytes = 0;
uint32_t Flash::programmedBytes = 0;
uint32_t Flash::reprogrammedBytes = 0;

//---------------------------------------------------------------------------

__attribute((weak)) void Flash::Erase(const void *target, size_t size) {
  erasedBytes += size;
  assert((size & (BLOCK_SIZE - 1)) == 0);
  memset((void *)target, 0xff, size);
}

__attribute((weak)) void Flash::Write(const void *target, const void *data,
                                      size_t size) {
  erasedBytes += size;
  programmedBytes += size;
  assert(target != data);
  assert((size & (BLOCK_SIZE - 1)) == 0);
  memcpy((void *)target, data, size);
}

//---------------------------------------------------------------------------

bool Flash::RequiresErase(const void *target, size_t size) {
  size_t count = size / sizeof(size_t);
  const size_t *t = (const size_t *)target;

  for (size_t i = 0; i < count; ++i) {
    if (t[i] != (size_t)-1) {
      return true;
    }
  }

  return false;
}

bool Flash::RequiresErase(const void *target, const void *data, size_t size) {
  size_t count = size / sizeof(size_t);
  const size_t *t = (const size_t *)target;
  const size_t *d = (const size_t *)data;

  for (size_t i = 0; i < count; ++i) {
    // Programming lowers a bit from 1 to 0, erasing raises it to 1.
    // Erasing is required if the data needs a 1, but the current state is a 0.
    if (d[i] & ~t[i]) {
      return true;
    }
  }

  return false;
}

bool Flash::RequiresProgram(const void *target, const void *data, size_t size) {
  return memcmp(target, data, size) != 0;
}

//---------------------------------------------------------------------------

void Flash::PrintInfo() {
  Console::Printf("Flash session statistics\n");
  Console::Printf("  Erased bytes: %u\n", Flash::erasedBytes);
  Console::Printf("  Programmed bytes: %u\n", Flash::programmedBytes);
  Console::Printf("  Reprogrammed bytes: %u\n", Flash::reprogrammedBytes);
}

//---------------------------------------------------------------------------

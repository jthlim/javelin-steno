//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class Flash {
public:
  // Sets all bits in the region to 1.
  static void Erase(const void *target, size_t size);

  static void Write(const void *target, const void *data, size_t size);

  static const size_t BLOCK_SIZE = 4096;

  static void PrintInfo();

  static uint32_t erasedBytes;
  static uint32_t programmedBytes;
  static uint32_t reprogrammedBytes;

private:
  static bool RequiresErase(const void *target, size_t size);
  static bool RequiresErase(const void *target, const void *data, size_t size);
  static bool RequiresProgram(const void *target, const void *data,
                              size_t size);
};

//---------------------------------------------------------------------------

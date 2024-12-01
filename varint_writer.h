//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct VarintWriter {
  VarintWriter(char *p) : p(p) {}

  void Write(uint32_t x);

  static uint32_t ZigZagEncode(int32_t value) {
    return (value << 1) ^ (value >> 31);
  }

  char *p;
};

//---------------------------------------------------------------------------

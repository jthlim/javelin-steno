//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class Random {
public:
  static uint32_t GenerateUint32();

  // Falls back to GenerateUint32 if no hardware version is available.
  static uint32_t GenerateHardwareUint32();
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "random.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

uint32_t Random::GenerateUint32() {
  return rand() ^ (rand() << 10) ^ (rand() << 20);
}

__attribute__((weak)) uint32_t Random::GenerateHardwareUint32() {
  return GenerateUint32();
}

//---------------------------------------------------------------------------

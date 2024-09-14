//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class JavelinHash {
public:
  static uint32_t Hash(const uint32_t *data, size_t wordCount);
  static constexpr uint32_t EmptyHash() {
    constexpr uint32_t s0 = prime0;
    constexpr uint32_t s1 = prime0 + prime1;
    constexpr uint32_t result = s0 ^ s1;
    return result;
  }

private:
  static constexpr uint32_t prime0 = 2654435761;
  static constexpr uint32_t prime1 = 2246822519;
};
//---------------------------------------------------------------------------

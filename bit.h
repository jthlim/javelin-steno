//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

template <size_t N> class Bit {};

template <> class Bit<4> {
public:
  static inline uint32_t CountTrailingZeros(uint32_t v) {
    return __builtin_ctz(v);
  }

  static inline uint32_t PopCount(uint32_t v) { return __builtin_popcount(v); }
};

template <> class Bit<8> {
public:
  static inline uint64_t CountTrailingZeros(uint64_t v) {
    return __builtin_ctzll(v);
  }

  static inline uint64_t PopCount(uint64_t v) {
    return __builtin_popcountll(v);
  }
};

//---------------------------------------------------------------------------

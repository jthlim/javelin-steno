//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

template <size_t N> class Bit {};

template <> class Bit<1> {
public:
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
  inline static uint32_t ReverseBits(uint32_t v) {
    __asm("rbit %0, %0" : "+r"(v) : :);
    return v >> 24;
  }
#else
  static uint32_t ReverseBits(uint32_t v);
#endif
};

template <> class Bit<4> {
public:
  static inline uint32_t CountTrailingZeros(uint32_t v) {
    return __builtin_ctz(v);
  }

#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
  inline static uint32_t ReverseBits(uint32_t v) {
    __asm("rbit %0, %0" : "+r"(v) : :);
    return v;
  }
#else
  static uint32_t ReverseBits(uint32_t v);
#endif

#if JAVELIN_USE_CUSTOM_POP_COUNT
  static uint32_t PopCount(uint32_t v);
#else
  static inline uint32_t PopCount(uint32_t v) { return __builtin_popcount(v); }
#endif
};

template <> class Bit<8> {
public:
  static inline uint32_t CountTrailingZeros(uint64_t v) {
    return __builtin_ctzll(v);
  }

  static inline uint32_t PopCount(uint64_t v) {
    return __builtin_popcountll(v);
  }
};

//---------------------------------------------------------------------------

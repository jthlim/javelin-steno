//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

class MapDataLookup {
public:
  MapDataLookup() = default;
  MapDataLookup(const uint8_t *p) : p(p) {}

  bool HasData() const { return *p != 0xff; }
  const void *GetData(const uint8_t *baseAddress) const {
    uint32_t offset = p[0] | (p[1] << 7) | (p[2] << 14) + (p[3] << 21);
    return baseAddress + offset;
  }

  void operator++() { p += 4; }

  const uint8_t *GetPointer() const { return p; }

  static const uint8_t *FindNextWordStart(const uint8_t *p) {
    while (*p != 0xff) {
      p += 4;
    }
    return p + 1;
  }

private:
  const uint8_t *p;
};

//---------------------------------------------------------------------------

struct StenoTextBlock {
#if JAVELIN_CPU_CORTEX_M4
  static uint32_t uqsub8(uint32_t a, uint32_t b) {
    uint32_t result;
    asm("uqsub8 %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    return result;
  }
#endif

  static const uint8_t *FindPreviousWordStart(const uint8_t *p) {
#if JAVELIN_CPU_CORTEX_M4
    uint32_t mask;
    do {
      p -= 4;
      const uint32_t v = *(const uint32_t *)p;
      mask = uqsub8(v, 0xfefefefe);
    } while (mask == 0);
    return (p + 4) - (__builtin_clz(mask) >> 3);
#else
    while (p[-1] != 0xff) {
      --p;
    }
    return p;
#endif
  }
  static const uint8_t *FindNextWordStart(const uint8_t *p) {
#if JAVELIN_CPU_CORTEX_M4
    uint32_t mask;
    do {
      const uint32_t v = *(const uint32_t *)p;
      p += 4;
      mask = uqsub8(v, 0xfefefefe);
    } while (mask == 0);
    return (p - 3) + (__builtin_clz(__builtin_bswap32((mask))) >> 3);
#else
    while (*p++ != 0xff) {
    }
    return p;
#endif
  }
};

//---------------------------------------------------------------------------

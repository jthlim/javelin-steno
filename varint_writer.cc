//---------------------------------------------------------------------------

#include "varint_writer.h"

//---------------------------------------------------------------------------

static inline void WriteUint16(char *p, uint32_t x) {
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
  *(uint16_t *)p = x;
#else
  p[0] = x;
  p[1] = x >> 8;
#endif
}

static inline void WriteUint32(char *p, uint32_t x) {
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
  *(uint32_t *)p = x;
#else
  p[0] = x;
  p[1] = x >> 8;
  p[2] = x >> 16;
  p[3] = x >> 24;
#endif
}

//---------------------------------------------------------------------------

void VarintWriter::Write(uint32_t x) {
  char *localP = p;
  if (x < 0x80) {
    *localP++ = x << 1;
  } else if (x < 0x4000) {
    WriteUint16(localP, (x << 2) | 1);
    localP += 2;
  } else if (x < 0x200000) {
#if JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33
    WriteUint32(localP, (x << 3) | 3);
    localP += 3;
#else
    *localP++ = (x << 3) | 3;
    *localP++ = x >> 5;
    *localP++ = x >> 13;
#endif
  } else if (x < 0x10000000) {
    WriteUint32(localP, (x << 4) | 7);
    localP += 4;
  } else {
    *localP++ = (x << 5) | 15;
    WriteUint32(localP, x >> 3);
    localP += 4;
  }
  p = localP;
}

//---------------------------------------------------------------------------

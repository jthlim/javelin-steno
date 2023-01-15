//---------------------------------------------------------------------------

#include "bit.h"

//---------------------------------------------------------------------------

#if JAVELIN_USE_CUSTOM_POP_COUNT

#if JAVELIN_ASSEMBLER_THUMB2
__attribute__((naked)) uint32_t Bit<4>::PopCount(uint32_t v) {
  asm volatile("ldr r1,=#0x49249249 \n\t"
               "lsr r2,r0,#1        \n\t"
               "and r2,r1           \n\t"
               "lsr r3,r0,#2        \n\t"
               "and r3,r1           \n\t"
               "and r0,r1           \n\t"
               "add r0,r2           \n\t"
               "add r0,r3           \n\t"
               "lsr r1,r0,#3        \n\t"
               "add r0,r1           \n\t"
               "ldr r1,=#0xC71C71C7 \n\t"
               "and r0,r1           \n\t"
               "lsr r1,r0,#6        \n\t"
               "add r0,r1           \n\t"
               "ldr r1,=#0x04004004 \n\t"
               "mul r0,r1           \n\t"
               "lsr r0,#26          \n\t"
               "bx lr               \n\t");
}
#else

uint32_t Bit<4>::PopCount(uint32_t v) {
  v = (v & 0x49249249) + ((v >> 1) & 0x49249249) + ((v >> 2) & 0x49249249);
  v = (v + (v >> 3)) & 0xC71C71C7;
  v += (v >> 6);
  return (v * 0x04004004) >> 26;
}

#endif

#endif

//---------------------------------------------------------------------------

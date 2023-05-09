//---------------------------------------------------------------------------

#include "bit.h"

//---------------------------------------------------------------------------

#if JAVELIN_USE_CUSTOM_POP_COUNT

#if JAVELIN_CPU_CORTEX_M0
__attribute__((naked)) uint32_t Bit<4>::PopCount(uint32_t v) {
  asm volatile(R"(
    ldr  r1, =#0x49249249
    lsr  r2, r0, #1
    and  r2, r1
    lsr  r3, r0, #2
    and  r3, r1
    and  r0, r1
    add  r0, r2
    add  r0, r3
    lsr  r1, r0, #3
    add  r0, r1
    ldr  r1, =#0xC71C71C7
    and  r0, r1
    lsr  r1, r0, #6
    add  r0, r1
    ldr  r1, =#0x04004004
    mul  r0, r1
    lsr  r0, #26
    bx   lr
  )");
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

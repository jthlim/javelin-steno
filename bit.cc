//---------------------------------------------------------------------------

#include "bit.h"

//---------------------------------------------------------------------------

#if JAVELIN_USE_CUSTOM_POP_COUNT

#if JAVELIN_CPU_CORTEX_M0
[[gnu::naked]] uint32_t Bit<4>::PopCount(uint32_t v) {
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
#elif JAVELIN_CPU_CORTEX_M4 || JAVELIN_CPU_CORTEX_M33

[[gnu::noinline]] uint32_t Bit<4>::PopCount(uint32_t v) {
  uint32_t temp;
  asm volatile(R"(
    bic %1, %0, #0x55555555
    sub %0, %0, %1, lsr #1
    bic %1, %0, #0x33333333
    and %0, %0, #0x33333333
    add %0, %0, %1, lsr #2
    add %0, %0, %0, lsr #4
    bic %1, %0, #0x0f0f0f0f
    usad8 %0, %0, %1
  )"
               : "+r"(v), "=r"(temp));
  return v;
}
#else

#error JAVELIN_USE_CUSTOM_POP_COUNT set but no implementation available.

#endif

#endif

//---------------------------------------------------------------------------

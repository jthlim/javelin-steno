//---------------------------------------------------------------------------

#include "mem.h"
#include <assert.h>
#include <stdint.h>

//---------------------------------------------------------------------------

bool AlignedMem<1>::ConstantTimeEq(const void *p, const void *q,
                                   size_t length) {
  void *pEnd = (char *)p + length;
  const uint8_t *bp = (const uint8_t *)p;
  const uint8_t *bq = (const uint8_t *)q;

  uint32_t delta = 0;
  while (bp < pEnd) {
    delta |= *bp++ ^ *bq++;
  }
  return delta == 0;
}

//---------------------------------------------------------------------------

void AlignedMem<sizeof(size_t)>::Clear(void *p, size_t length) {
  assert(length % sizeof(size_t) == 0);

#if JAVELIN_CPU_CORTEX_M4
  register int r1 __asm__("r2") = 0;
  register int r2 __asm__("r3") = 0;
  register int r3 __asm__("r4") = 0;
  register int r4 __asm__("r5") = 0;
  while (length >= 32) {
    asm volatile("stmia %0!, {%1, %2, %3, %4}"
                 : "+l"(p)
                 : "l"(r1), "l"(r2), "l"(r3), "l"(r4)
                 : "memory");
    asm volatile("stmia %0!, {%1, %2, %3, %4}"
                 : "+l"(p)
                 : "l"(r1), "l"(r2), "l"(r3), "l"(r4)
                 : "memory");
    length -= 32;
  }
  if (length & 16) {
    asm volatile("stmia %0!, {%1, %2, %3, %4}"
                 : "+l"(p)
                 : "l"(r1), "l"(r2), "l"(r3), "l"(r4)
                 : "memory");
  }
  if (length & 8) {
    asm volatile("stmia %0!, {%1, %2}" : "+l"(p) : "l"(r1), "l"(r2) : "memory");
  }
  if (length & 4) {
    asm volatile("str %1, [%0]" : : "l"(p), "l"(r1) : "memory");
  }
#else
  void *pEnd = (char *)p + length;
  size_t *sp = (size_t *)p;

  while (sp < pEnd) {
    *sp++ = 0;
  }
#endif
}

bool AlignedMem<sizeof(size_t)>::ConstantTimeEq(const void *p, const void *q,
                                                size_t length) {
  assert(length % sizeof(size_t) == 0);

  void *pEnd = (char *)p + length;
  const size_t *sp = (const size_t *)p;
  const size_t *sq = (const size_t *)q;

  size_t delta = 0;
  while (sp < pEnd) {
    delta |= *sp++ ^ *sq++;
  }
  return delta == 0;
}

void AlignedMem<sizeof(size_t)>::Copy(void *d, const void *s, size_t length) {
  assert(length % sizeof(size_t) == 0);

#if JAVELIN_CPU_CORTEX_M4
  while (length >= 32) {
    asm volatile("vldmia %0!, {s0-s7}\n\t"
                 "vstmia %1!, {s0-s7}\n\t"
                 : "+r"(s), "+r"(d)
                 :
                 : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "memory");
    length -= 32;
  }
  if (length & 16) {
    asm volatile("vldmia %0!, {s0-s3}\n\t"
                 "vstmia %1!, {s0-s3}\n\t"
                 : "+r"(s), "+r"(d)
                 :
                 : "s0", "s1", "s2", "s3", "memory");
  }
  if (length & 8) {
    asm volatile("vldmia %0!, {s0-s1}\n\t"
                 "vstmia %1!, {s0-s1}\n\t"
                 : "+r"(s), "+r"(d)
                 :
                 : "s0", "s1", "memory");
  }
  if (length & 4) {
    int temp;
    asm volatile("ldr %0, [%1]" : "=l"(temp) : "l"(s));
    asm volatile("str %0, [%1]" : : "l"(temp), "l"(d) : "memory");
  }
#else
  void *sEnd = (char *)s + length;
  const size_t *ss = (const size_t *)s;
  size_t *sd = (size_t *)d;

  while (ss < sEnd) {
    *sd++ = *ss++;
  }
#endif
}

bool AlignedMem<sizeof(size_t)>::Eq(const void *p, const void *q,
                                    size_t length) {
  assert(length % sizeof(size_t) == 0);

  void *pEnd = (char *)p + length;
  const size_t *sp = (const size_t *)p;
  const size_t *sq = (const size_t *)q;

  while (sp < pEnd) {
    if (*sp++ != *sq++) {
      return false;
    }
  }
  return true;
}

void AlignedMem<sizeof(size_t)>::Fill(void *p, size_t length) {
  assert(length % sizeof(size_t) == 0);

  void *pEnd = (char *)p + length;
  size_t *sp = (size_t *)p;

  while (sp < pEnd) {
    *sp++ = (size_t)-1;
  }
}

//---------------------------------------------------------------------------

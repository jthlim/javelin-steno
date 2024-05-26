//---------------------------------------------------------------------------

#include "mem.h"
#include <assert.h>

//---------------------------------------------------------------------------

bool AlignedMem<1>::ConstantTimeEq(const void *p, const void *q,
                                   size_t length) {
  void *pEnd = (char *)p + length;
  const uint8_t *bp = (const uint8_t *)p;
  const uint8_t *bq = (const uint8_t *)q;

  uint8_t delta = 0;
  while (bp < pEnd) {
    delta |= *bp++ ^ *bq++;
  }
  return delta == 0;
}

//---------------------------------------------------------------------------

void AlignedMem<sizeof(size_t)>::Clear(void *p, size_t length) {
  assert(length % sizeof(size_t) == 0);

  void *pEnd = (char *)p + length;
  size_t *sp = (size_t *)p;

  while (sp < pEnd) {
    *sp++ = 0;
  }
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

  void *sEnd = (char *)s + length;
  const size_t *ss = (const size_t *)s;
  size_t *sd = (size_t *)d;

  while (ss < sEnd) {
    *sd++ = *ss++;
  }
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

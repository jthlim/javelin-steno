//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

//---------------------------------------------------------------------------

// XipPointer is a helper class to avoid nrf5 anomaly 216.
//
// Anomaly 216 causes loads from XIP region to return incorrect data if
// the address was recently loaded from XIP:
//   ldr  r0, [r1]      // r1 points to XIP
//   ldr  r2, [r0]      // r0 points to XIP, issue occurs.
//
// It has also been observed to occur when a read occurs from XIP and the
// address is updated, even when the pointer itself was not from XIP:
//  ldr   r0, [r1], #4  // r1 points to XIP, and is advanced 4 bytes.
//  ldr   r2, [r1]      // Issue occurs here.
template <typename T> class XipPointer {
public:
  XipPointer() = default;
  constexpr XipPointer(const T *p) : p(p) {}

  operator const T *() const {
    const T *result = p;
#if defined(JAVELIN_PLATFORM_NRF5_SDK)
    asm volatile("dsb");
#endif
    return result;
  }

  void operator+=(size_t t) { p += t; }

private:
  const T *p;
};

//---------------------------------------------------------------------------

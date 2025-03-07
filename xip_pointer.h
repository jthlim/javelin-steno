//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

//---------------------------------------------------------------------------

// XipPointer is a helper class to avoid nrf5 anomaly 216.
//
// Anomaly 216 causes loads from XIP region to return incorrect data if
// the address was recently loaded from XIP:
//   ldr  r0, [r1]      // r1 points to XIP
//   ldr  r2, [r0]      // r0 points to XIP, issue occurs.
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

  const T *operator->() const {
    const T *p = (*this);
    return p;
  }

  XipPointer operator+(int t) { return XipPointer(p + t); }
  XipPointer operator+(size_t t) { return XipPointer(p + t); }
  void operator+=(size_t t) { p += t; }

  XipPointer &operator++() {
    ++p;
    return *this;
  }
  XipPointer &operator--() {
    --p;
    return *this;
  }
  XipPointer operator++(int) { return XipPointer(p++); }
  XipPointer operator--(int) { return XipPointer(p--); }

private:
  const T *p;
};

//---------------------------------------------------------------------------

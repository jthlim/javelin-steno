//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

//---------------------------------------------------------------------------

// using new(std::nothrow) adds more than 50kb to the rp2040 binary.
// Define a custom NoThrow to avoid that bloat.
struct NoThrow {};

//---------------------------------------------------------------------------

inline void *operator new(size_t size, const NoThrow &) noexcept {
  return malloc(size);
}
inline void operator delete(void *p, const NoThrow &) noexcept { free(p); }

//---------------------------------------------------------------------------

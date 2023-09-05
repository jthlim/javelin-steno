//---------------------------------------------------------------------------

#pragma once
#include <stdlib.h>

//---------------------------------------------------------------------------

// Class used to provide automatic nothrow semantics, without the 50kb code
// bloat of using std::nothrow.
class JavelinMallocAllocate {
public:
  constexpr JavelinMallocAllocate() = default;

  static inline void *operator new(size_t size) noexcept {
    return malloc(size);
  }
  static inline void operator delete(void *p) noexcept { free(p); };
};

//---------------------------------------------------------------------------

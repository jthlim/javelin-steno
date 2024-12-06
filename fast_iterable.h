//---------------------------------------------------------------------------

#pragma once
#include "iterable.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

// FastIterable stores a start and end pointer, so that the calculation
//   end = start + count
// is not required at the start of each loop.
template <typename T> class FastIterable {
public:
  template <typename S> FastIterable(S &s) : p(begin(s)), pEnd(end(s)) {}

  friend T *begin(const FastIterable &it) { return it.p; }
  friend T *end(const FastIterable &it) { return it.pEnd; }

  ReverseIterableData<T> Reverse() {
    return ReverseIterableData<T>{
        .count = size_t(pEnd - p),
        .data = p,
    };
  }
  ReverseIterableData<const T> Reverse() const {
    return ReverseIterableData<const T>{
        .count = size_t(pEnd - p),
        .data = p,
    };
  }

private:
  T *p;
  T *pEnd;
};

//---------------------------------------------------------------------------

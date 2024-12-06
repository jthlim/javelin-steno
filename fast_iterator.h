//---------------------------------------------------------------------------

#pragma once
#include "iterable.h"
#include <stdlib.h>

//---------------------------------------------------------------------------

// FastIterator stores a start and end pointer, so that the calculation
//   end = start + count
// is not required at the start of each loop.
template <typename T> class FastIterator {
public:
  template <typename S> FastIterator(S &s) : p(begin(s)), pEnd(end(s)) {}

  friend T *begin(const FastIterator &it) { return it.p; }
  friend T *end(const FastIterator &it) { return it.pEnd; }

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

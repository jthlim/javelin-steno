//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

template <typename T> class JavelinStaticAllocate {
public:
  JavelinStaticAllocate() {}

  union {
    uint8_t storage[sizeof(T)];
    T value;
  };

  inline operator T &() { return value; }
  inline T *operator->() { return &value; }
};

template <typename T>
inline void *operator new(size_t n, JavelinStaticAllocate<T> &p) {
  assert(n == sizeof(T));
  return p.storage;
}

//---------------------------------------------------------------------------

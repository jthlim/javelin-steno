//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stddef.h>

//---------------------------------------------------------------------------

template <typename T> union JavelinStaticAllocate {
public:
  JavelinStaticAllocate() {}
  ~JavelinStaticAllocate() {}

  T value;

  inline operator T &() { return value; }
  inline T *operator->() { return &value; }
};

template <typename T>
inline void *operator new(size_t n, JavelinStaticAllocate<T> &p) {
  assert(n == sizeof(T));
  return &p.value;
}

//---------------------------------------------------------------------------

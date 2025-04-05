//---------------------------------------------------------------------------

#pragma once
#include "../mem.h"
#include "iterable.h"
#include <assert.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

template <typename T> class SizedList {
public:
  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count != 0; }
  size_t GetCount() const { return count; }
  void SetCount(size_t newCount) { count = newCount; }

  T &operator[](size_t i) {
    assert(i < count);
    return ((T *)data)[i];
  }
  const T &operator[](size_t i) const {
    assert(i < count);
    return data[i];
  }

  T &Front() { return ((T *)data)[0]; }
  T &Back() { return ((T *)data)[count - 1]; }
  const T &Front() const { return data[0]; }
  const T &Back() const { return data[count - 1]; }

  friend const T *begin(const SizedList &list) { return list.data; }
  friend const T *end(const SizedList &list) { return list.data + list.count; }

  IterableData<T> Skip(size_t n) {
    return IterableData<T>{.count = count - n, .data = (T *)data + n};
  }

  SizedList Skip(size_t n) const {
    return SizedList{.count = count - n, .data = data + n};
  }

  SizedList Copy() const {
    T *copy = (T *)malloc(sizeof(T) * count);
    Mem::Copy(copy, data, sizeof(T) * count);
    return SizedList{.count = count, .data = copy};
  }

  template <typename S> SizedList<S> Cast() const {
    return SizedList<S>{.count = count, .data = (S *)data};
  }

  static SizedList CreateWithCapacity(size_t n) {
    return SizedList{.count = 0, .data = (T *)malloc(sizeof(T) * n)};
  }
  void Add(const T &v) { ((T *)data)[count++] = v; }
  void Pop() { --count; }

  size_t count;
  const T *data;
};

//---------------------------------------------------------------------------

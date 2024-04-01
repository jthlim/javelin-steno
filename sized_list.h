//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

template <typename T> class SizedList {
public:
  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count != 0; }
  size_t GetCount() const { return count; }

  const T &operator[](size_t i) const {
    assert(i < count);
    return data[i];
  }

  const T &Front() const { return data[0]; }
  const T &Back() const { return data[count - 1]; }

  friend const T *begin(const SizedList &list) { return list.data; }
  friend const T *end(const SizedList &list) { return list.data + list.count; }

  SizedList Copy() const {
    void *copy = malloc(sizeof(T) * count);
    memcpy(copy, data, sizeof(T) * count);
    return SizedList{.count = count, .data = (T *)copy};
  }

  template <typename S> SizedList<S> Cast() const {
    return SizedList<S>{.count = count, .data = (S *)data};
  }

  static SizedList CreateWithCapacity(size_t n) {
    return SizedList{.count = 0, .data = (T *)malloc(sizeof(T) * n)};
  }
  void Add(T v) { ((T *)data)[count++] = v; }

  size_t count;
  const T *data;
};

//---------------------------------------------------------------------------

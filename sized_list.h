//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stdlib.h>

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

  const size_t count;
  const T *const data;
};

//---------------------------------------------------------------------------

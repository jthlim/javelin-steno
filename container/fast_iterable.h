//---------------------------------------------------------------------------

#pragma once
#include "../interval.h"
#include "iterable.h"
#include <stddef.h>

//---------------------------------------------------------------------------

// FastIterable stores a start and end pointer, so that the calculation
//   end = start + count
// is not required at the start of each loop.
template <typename T> class FastIterable : protected Interval<T *> {
private:
  using super = Interval<T *>;

public:
  FastIterable() = default;
  FastIterable(T *p, T *pEnd) : super(p, pEnd) {}
  template <size_t N> FastIterable(T (&t)[N]) : super(t, (T *)t + N) {}
  template <typename S> FastIterable(S &s) : super(begin(s), end(s)) {}
  template <typename S> FastIterable(S &&s) : super(begin(s), end(s)) {
    s.HasMoved();
  }

  bool IsEmpty() const { return super::min == super::max; }
  bool IsNotEmpty() const { return super::min != super::max; }
  size_t GetCount() const { return super::max - super::min; }

  T &Front() { return super::min[0]; }
  T &Back() { return super::max[-1]; }

  friend T *begin(const FastIterable &it) { return it.min; }
  friend T *end(const FastIterable &it) { return it.max; }

  ReverseIterableData<T> Reverse() {
    return ReverseIterableData<T>{.count = GetCount(), .data = super::min};
  }
  ReverseIterableData<const T> Reverse() const {
    return ReverseIterableData<const T>{
        .count = GetCount(),
        .data = super::min,
    };
  }
};

//---------------------------------------------------------------------------

template <typename T, size_t CAPACITY>
class FastIterableStaticList : public FastIterable<T> {
private:
  using super = FastIterable<T>;

public:
  FastIterableStaticList() : super(data, data) {}

  void Add(T t) { *super::max++ = t; }
  bool IsFull() const { return super::max == data + CAPACITY; }

private:
  T data[CAPACITY];
};

//---------------------------------------------------------------------------

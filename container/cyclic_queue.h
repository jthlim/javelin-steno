//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stddef.h>

//---------------------------------------------------------------------------

template <typename T, size_t N> class CyclicQueue {
private:
  template <typename IT> struct Iterator {
    size_t i;
    IT *data;

    bool operator!=(size_t endIndex) const { return i != endIndex; }
    void operator++() { ++i; }
    IT &operator*() const { return data[i & (N - 1)]; }
  };

public:
  CyclicQueue() { static_assert((N & (N - 1)) == 0, "N must be a power of 2"); }

  void Reset() {
    startIndex = 0;
    endIndex = 0;
  }

  bool IsEmpty() const { return startIndex == endIndex; }
  bool IsNotEmpty() const { return startIndex != endIndex; }
  bool IsFull() const { return endIndex - startIndex == N; }
  bool IsNotFull() const { return endIndex - startIndex != N; }
  size_t GetCount() const { return endIndex - startIndex; }
  size_t GetCapacity() const { return N; }
  size_t GetAvailable() const { return N - GetCount(); }

  T &Add() { return data[endIndex++ & (N - 1)]; }
  void Add(const T &value) { data[endIndex++ & (N - 1)] = value; }

  T &RemoveFront() { return data[startIndex++ & (N - 1)]; }
  void RemoveFront(size_t count) {
    assert(GetCount() >= count);
    startIndex += count;
  }

  T &RemoveBack() { return data[--endIndex & (N - 1)]; }
  void RemoveBack(size_t count) {
    assert(GetCount() >= count);
    endIndex -= count;
  }

  T &Back(size_t fromEnd = 1) { return data[(endIndex - fromEnd) & (N - 1)]; }
  const T &Back(size_t fromEnd = 1) const {
    return data[(endIndex - fromEnd) & (N - 1)];
  }
  T &Front() { return data[startIndex & (N - 1)]; }
  const T &Front() const { return data[startIndex & (N - 1)]; }

  T &operator[](size_t n) { return data[(startIndex + n) & (N - 1)]; }
  const T &operator[](size_t n) const {
    return data[(startIndex + n) & (N - 1)];
  }

  friend Iterator<T> begin(CyclicQueue &q) {
    return Iterator{.i = q.startIndex, .data = q.data};
  }
  friend Iterator<const T> begin(const CyclicQueue &q) {
    return Iterator{.i = q.startIndex, .data = q.data};
  }
  friend size_t end(const CyclicQueue &q) { return q.endIndex; }

private:
  size_t startIndex = 0;
  size_t endIndex = 0;

  T data[N];
};

//---------------------------------------------------------------------------

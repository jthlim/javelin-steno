//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

template <typename T, size_t N> class CyclicQueue {
public:
  CyclicQueue() { static_assert((N & (N - 1)) == 0, "N must be a power of 2"); }

  void Reset() {
    start = 0;
    end = 0;
  }

  bool IsEmpty() const { return start == end; }
  bool IsNotEmpty() const { return start != end; }
  bool IsFull() const { return end - start == N; }
  bool IsNotFull() const { return end - start != N; }
  size_t GetCount() const { return end - start; }
  size_t GetAvailable() const { return N - GetCount(); }

  T &Add() { return data[end++ & (N - 1)]; }
  void Add(const T &value) { data[end++ & (N - 1)] = value; }

  T &RemoveFront() { return data[start++ & (N - 1)]; }
  void RemoveFront(size_t count) {
    assert(GetCount() >= count);
    start += count;
  }

  T &RemoveBack() { return data[--end & (N - 1)]; }
  void RemoveBack(size_t count) {
    assert(GetCount() >= count);
    end -= count;
  }

  T &Back(size_t fromEnd = 1) { return data[(end - fromEnd) & (N - 1)]; }
  const T &Back(size_t fromEnd = 1) const {
    return data[(end - fromEnd) & (N - 1)];
  }
  T &Front() { return data[start & (N - 1)]; }
  const T &Front() const { return data[start & (N - 1)]; }

  T &operator[](size_t n) { return data[(start + n) & (N - 1)]; }
  const T &operator[](size_t n) const { return data[(start + n) & (N - 1)]; }

private:
  size_t start = 0;
  size_t end = 0;

  T data[N];
};

//---------------------------------------------------------------------------

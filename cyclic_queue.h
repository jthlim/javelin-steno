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

  bool HasData() const { return start != end; }
  bool IsEmpty() const { return start == end; }
  bool IsFull() const { return end - start == N; }
  size_t GetCount() const { return end - start; }
  size_t GetAvailable() const { return N - GetCount(); }

  T &Add() { return data[end++ & (N - 1)]; }
  void Add(const T &value) { data[end++ & (N - 1)] = value; }

  T &RemoveFront() { return data[start++ & (N - 1)]; }

  T &Back() { return data[(end - 1) & (N - 1)]; }
  const T &Back() const { return data[(end - 1) & (N - 1)]; }
  T &Front() { return data[start & (N - 1)]; }
  const T &Front() const { return data[start & (N - 1)]; }

private:
  size_t start = 0;
  size_t end = 0;

  T data[N];
};

//---------------------------------------------------------------------------

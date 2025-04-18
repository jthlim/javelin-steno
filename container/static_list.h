//---------------------------------------------------------------------------

#pragma once
#include "sized_list.h"
#include <assert.h>
#include <stddef.h>

//---------------------------------------------------------------------------

template <typename T, size_t CAPACITY = 0> class StaticList {
public:
  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count != 0; }
  bool IsFull() const { return count == CAPACITY; }
  bool IsNotFull() const { return count < CAPACITY; }
  bool CanAddCount(size_t n) const { return count + n <= CAPACITY; }
  size_t GetCount() const { return count; }
  void SetCount(size_t value) { count = value; }
  size_t GetCapacity() const { return CAPACITY; }
  size_t GetRemainingCapacity() const { return CAPACITY - count; }
  void Reset() { count = 0; }

  T &Add() {
    assert(count < CAPACITY);
    return data[count++];
  }
  void Add(const T &value) {
    assert(count < CAPACITY);
    data[count++] = value;
  }
  void AddCount(size_t n) {
    count += n;
    assert(count <= CAPACITY);
  }

  void PopFront() {
    assert(count > 0);
    memmove(&data[0], &data[1], sizeof(T) * --count);
  }
  void PopFrontCount(size_t n) {
    assert(n <= count);
    count -= n;
    memmove(&data[0], &data[n], sizeof(T) * count);
  }
  T &PopBack() {
    assert(count > 0);
    return data[--count];
  }

  T &operator[](size_t i) {
    assert(i < count);
    return data[i];
  }
  const T &operator[](size_t i) const {
    assert(i < count);
    return data[i];
  }

  T &Front() { return data[0]; }
  const T &Front() const { return data[0]; }
  T &Back() { return data[count - 1]; }
  const T &Back() const { return data[count - 1]; }

  SizedList<T> Skip(size_t n) {
    return SizedList<T>{.count = count - n, .data = data + n};
  }
  SizedList<const T> Skip(size_t n) const {
    return SizedList<const T>{.count = count - n, .data = data + n};
  }

  void Sort(int (*comparator)(const T *a, const T *b)) {
    qsort(data, count, sizeof(T),
          (int (*)(const void *, const void *))comparator);
  }

  friend const T *begin(const StaticList &list) { return list.data; }
  friend const T *end(const StaticList &list) { return list.data + list.count; }

  friend T *begin(StaticList &list) { return list.data; }
  friend T *end(StaticList &list) { return list.data + list.count; }

  void Remove(T v) {
    for (size_t i = 0; i < count; ++i) {
      if (data[i] == v) {
        RemoveIndex(i);
        return;
      }
    }
  }

  void RemoveIndex(size_t i) {
    --count;
    for (; i < count; ++i) {
      data[i] = data[i + 1];
    }
  }

private:
  size_t count = 0;
  T data[CAPACITY];
};

//---------------------------------------------------------------------------

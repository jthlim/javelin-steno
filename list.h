//---------------------------------------------------------------------------

#pragma once
#include "iterable.h"
#include "malloc_allocate.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------

class _ListBase : public JavelinMallocAllocate {
public:
  ~_ListBase() { free(buffer); }
  _ListBase(_ListBase &&other) {
    buffer = other.buffer;
    count = other.count;
    other.buffer = nullptr;
    other.count = 0;
  }

  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count != 0; }
  size_t GetCount() const { return count; }
  void Reset() {
    uint8_t *toFree = buffer;
    count = 0;
    buffer = nullptr;
    free(toFree);
  }

protected:
  _ListBase() : count(0), buffer(nullptr) {}

  void Add(const void *data, size_t elementSize);
  void AddCount(const void *data, size_t n, size_t elementSize);
  void InsertAt(const void *data, size_t index, size_t elementSize);

  size_t count;
  uint8_t *buffer;

  static size_t GetCapacity(size_t count);

private:
  _ListBase(const _ListBase &) = delete;
};

//---------------------------------------------------------------------------

// Optimized for code size. Only usable with PODs.
template <typename T> class List : public _ListBase {
private:
  using super = _ListBase;

public:
  List() = default;
  List(List &&other) : super((super &&)other) {}

  void Add(const T &v) { super::Add(&v, sizeof(T)); }
  void AddCount(const T *v, size_t n) { super::AddCount(v, n, sizeof(T)); }
  template <typename D> void AddCount(const IterableData<D> &data) {
    AddCount(data.data, data.count);
  }
  void AddIfUnique(const T &v) {
    if (!Contains(v)) {
      Add(v);
    }
  }

  void InsertAt(size_t i, const T &v) { super::InsertAt(&v, i, sizeof(T)); }

  void Sort(int (*comparator)(const T *, const T *)) {
    qsort(buffer, count, sizeof(T),
          (int (*)(const void *, const void *))comparator);
  }

  void Pop() { --count; }

  bool Contains(const T &v) const {
    for (const T &x : *this) {
      if (x == v) {
        return true;
      }
    }
    return false;
  }

  T &operator[](size_t i) {
    assert(i < count);
    return ((T *)buffer)[i];
  }
  const T &operator[](size_t i) const {
    assert(i < count);
    return ((const T *)buffer)[i];
  }
  T &Front() { return (*this)[0]; }
  const T &Front() const { return (*this)[0]; }
  T &Back() { return (*this)[count - 1]; }
  const T &Back() const { return (*this)[count - 1]; }

  IterableData<T> Skip(size_t n) {
    return IterableData<T>{
        .count = count - n,
        .data = begin(*this) + n,
    };
  }
  IterableData<const T> Skip(size_t n) const {
    return IterableData<const T>{
        .count = count - n,
        .data = begin(*this) + n,
    };
  }
  ReverseIterableData<T> Reverse() {
    return ReverseIterableData<T>{
        .count = count,
        .data = begin(*this),
    };
  }
  ReverseIterableData<const T> Reverse() const {
    return ReverseIterableData<const T>{
        .count = count,
        .data = begin(*this),
    };
  }

  friend const T *begin(const List &list) { return (const T *)list.buffer; }
  friend const T *end(const List &list) { return begin(list) + list.count; }

  friend T *begin(List &list) { return (T *)list.buffer; }
  friend T *end(List &list) { return begin(list) + list.count; }
};

//---------------------------------------------------------------------------

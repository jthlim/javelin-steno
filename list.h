//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class _ListBase {
public:
  ~_ListBase() { free(buffer); }

  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count != 0; }
  size_t GetCount() const { return count; }

protected:
  _ListBase() : count(0), buffer(nullptr) {}

  void Add(const void *data, size_t elementSize);

  void Sort(int (*comparator)(const void *a, const void *b),
            size_t elementSize) {
    qsort(buffer, count, elementSize, comparator);
  }

  size_t count;
  uint8_t *buffer;

  static size_t GetCapacity(size_t count);
};

//---------------------------------------------------------------------------

// Optimized for code size. Only usable with PODs.
template <typename T> class List : public _ListBase {
public:
  List() {}

  void Add(const T &v) { _ListBase::Add(&v, sizeof(T)); }

  // Wanted this to be const T *a, const T *b, but running into
  // type conversion issues.
  void Sort(int (*comparator)(const void *a, const void *b)) {
    _ListBase::Sort(comparator, sizeof(T));
  }

  void Pop() { --count; }

  T &operator[](size_t i) {
    assert(i < count);
    return ((T *)buffer)[i];
  }
  const T &operator[](size_t i) const {
    assert(i < count);
    return ((const T *)buffer)[i];
  }
  T &Back() { return (*this)[count - 1]; }
};

//---------------------------------------------------------------------------

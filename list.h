//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

class _ListBase {
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

  void Sort(int (*comparator)(const void *a, const void *b),
            size_t elementSize) {
    qsort(buffer, count, elementSize, comparator);
  }

  size_t count;
  uint8_t *buffer;

  static size_t GetCapacity(size_t count);

private:
  _ListBase(const _ListBase &) = delete;
};

//---------------------------------------------------------------------------

// Optimized for code size. Only usable with PODs.
template <typename T> class List : public _ListBase {
public:
  List() = default;
  List(List &&other) : _ListBase((_ListBase &&) other) {}

  void Add(const T &v) { _ListBase::Add(&v, sizeof(T)); }

  // Wanted this to be const T *a, const T *b, but running into
  // type conversion issues.
  void Sort(int (*comparator)(const void *a, const void *b)) {
    _ListBase::Sort(comparator, sizeof(T));
  }

  void Pop() { --count; }

  bool Contains(const T &v) const {
    for (size_t i = 0; i < count; ++i) {
      if ((*this)[i] == v) {
        return true;
      }
    }
    return false;
  }

  T *GetData() const { return (T *)buffer; }

  T &operator[](size_t i) {
    assert(i < count);
    return ((T *)buffer)[i];
  }
  const T &operator[](size_t i) const {
    assert(i < count);
    return ((const T *)buffer)[i];
  }
  T &Back() { return (*this)[count - 1]; }
  const T &Back() const { return (*this)[count - 1]; }
};

//---------------------------------------------------------------------------

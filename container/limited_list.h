//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

//---------------------------------------------------------------------------

// A limited list is a list that has a maximum size.
// When the list is full and a new element is added,
// the oldest element is removed.
template <typename T, size_t CAPACITY> class LimitedList {
public:
  bool IsEmpty() const { return count == 0; }
  bool IsNotEmpty() const { return count == 0; }

  // Returns the number of elements contained in the list.
  size_t GetCount() const { return (count > CAPACITY) ? CAPACITY : count; }

  // Returns the total number of elements added to the list.
  size_t GetTotalCount() const { return count; }

  void Add(const T &t) { data[count++ & (CAPACITY - 1)] = t; }

  T &Back(size_t offset = 1) { return data[(count - offset) & (CAPACITY - 1)]; }
  const T &Back(size_t offset = 1) const {
    return data[(count - offset) & (CAPACITY - 1)];
  }

  const T &operator[](size_t i) const {
    if (count < CAPACITY) [[unlikely]] {
      return data[i];
    }
    return data[(count + i) & (CAPACITY - 1)];
  }

private:
  size_t count = 0;
  T data[CAPACITY];
};

//---------------------------------------------------------------------------

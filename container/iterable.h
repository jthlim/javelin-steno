//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

//---------------------------------------------------------------------------

template <typename T> class IterableData {
public:
  size_t count;
  T *data;

  friend T *begin(const IterableData &list) { return list.data; }
  friend T *end(const IterableData &list) { return list.data + list.count; }
};

//---------------------------------------------------------------------------

template <typename T> class ReverseIterableData {
private:
  class Iterator {
  public:
    Iterator(T *p) : p(p) {}

    T &operator*() const { return p[-1]; }
    void operator++() { --p; }

    bool operator!=(T *end) const { return p != end; }

  private:
    T *p;
  };

public:
  size_t count;
  T *data;

  friend Iterator begin(const ReverseIterableData &range) {
    return Iterator(range.data + range.count);
  }
  friend T *end(const ReverseIterableData &range) { return range.data; }
};

//---------------------------------------------------------------------------

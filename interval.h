//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T> struct Interval {
  T min;
  T max;

  void Set(T min, T max) {
    this->min = min;
    this->max = max;
  }

  bool Contains(T value) const { return min <= value && value < max; }
};

//---------------------------------------------------------------------------
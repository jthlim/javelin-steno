//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T> struct Interval {
  T min;
  T max;

  Interval() = default;
  Interval(T min, T max) : min(min), max(max) {}

  void Set(T min, T max) {
    this->min = min;
    this->max = max;
  }

  bool Contains(T value) const { return min <= value && value < max; }
};

//---------------------------------------------------------------------------
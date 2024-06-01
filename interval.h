//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T> struct Interval {
  T min;
  T max;

  bool Contains(T value) const { return min <= value && value < max; }
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T, typename U, typename V> T Clamp(T value, U min, V max) {
  if (value < min) {
    return min;
  }
  if (value > max) {
    return max;
  }
  return value;
}

template <typename T, typename U> T ClampMin(T value, U min) {
  return value < min ? min : value;
}

template <typename T, typename U> T ClampMax(T value, U max) {
  return value > max ? max : value;
}

//---------------------------------------------------------------------------

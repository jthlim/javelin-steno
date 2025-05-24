//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T, int N> class FixedPoint {
public:
  class Raw;
  static const Raw raw;

  constexpr FixedPoint() = default;
  constexpr FixedPoint(int value) : value(value << N) {}
  constexpr FixedPoint(unsigned int value) : value(value << N) {}
  consteval FixedPoint(double value) : value((T)(value * (1 << N))) {}
  constexpr FixedPoint(T value, const Raw &) : value(value) {}

  constexpr int GetIntegral() const { return value >> N; }
  constexpr FixedPoint GetFractional() const {
    return FixedPoint(value & ((1 << N) - 1), raw);
  }

  constexpr bool operator==(const FixedPoint &other) const {
    return value == other.value;
  }

  constexpr bool operator<(const FixedPoint &o) const {
    return value < o.value;
  }
  constexpr bool operator<=(const FixedPoint &o) const {
    return value <= o.value;
  }
  constexpr bool operator>(const FixedPoint &o) const {
    return value > o.value;
  }
  constexpr bool operator>=(const FixedPoint &o) const {
    return value >= o.value;
  }

  constexpr FixedPoint operator+(const FixedPoint &o) const {
    return FixedPoint(value + o.value, raw);
  }
  constexpr FixedPoint operator-(const FixedPoint &o) const {
    return FixedPoint(value - o.value, raw);
  }
  constexpr FixedPoint operator*(const FixedPoint &o) const {
    return FixedPoint((value * o.value) >> N, raw);
  }
  constexpr FixedPoint operator*(unsigned int o) const {
    return FixedPoint(value * o, raw);
  }
  constexpr FixedPoint operator/(const FixedPoint &o) const {
    return FixedPoint((value << N) / o.value, raw);
  }
  constexpr FixedPoint operator/(unsigned int o) const {
    return FixedPoint(value / o, raw);
  }

  void operator+=(const FixedPoint &o) { value += o.value; }
  void operator-=(const FixedPoint &o) { value -= o.value; }

  T value;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

template <typename T, int N> class FixedPoint {
public:
  constexpr FixedPoint() = default;
  constexpr FixedPoint(int value) : value(value << N) {}
  constexpr FixedPoint(unsigned int value) : value(value << N) {}
  consteval FixedPoint(double value) : value((T)(value * (1 << N))) {}

  constexpr int GetIntegral() const { return value >> N; }
  constexpr FixedPoint GetFractional() const {
    return FixedPoint(value & ((1 << N) - 1), true);
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
    return FixedPoint(value + o.value, true);
  }
  constexpr FixedPoint operator-(const FixedPoint &o) const {
    return FixedPoint(value - o.value, true);
  }
  constexpr FixedPoint operator*(const FixedPoint &o) const {
    return FixedPoint((value * o.value) >> N, true);
  }
  constexpr FixedPoint operator*(unsigned int o) const {
    return FixedPoint(value * o, true);
  }
  constexpr FixedPoint operator/(const FixedPoint &o) const {
    return FixedPoint((value << N) / o.value, true);
  }
  constexpr FixedPoint operator/(unsigned int o) const {
    return FixedPoint(value / o, true);
  }

  void operator+=(const FixedPoint &o) { value += o.value; }
  void operator-=(const FixedPoint &o) { value -= o.value; }

  T value;

private:
  constexpr FixedPoint(T value, bool) : value(value) {}
};

//---------------------------------------------------------------------------

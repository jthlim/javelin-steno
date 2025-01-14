//---------------------------------------------------------------------------

#pragma once
#include "bit.h"
#include "mem.h"

//---------------------------------------------------------------------------

class BitFieldIterator {
public:
  BitFieldIterator(const size_t *p, const size_t *end) : p(p), end(end) {
    UpdateValue();
  }

  bool operator!=(size_t _) const { return value != 0; }

  void operator++() {
    // Pull off lowest bit.
    value = value & (value - 1);
    UpdateValue();
  }

  size_t operator*() const {
    return Bit<sizeof(size_t)>::CountTrailingZeros(value) + bitOffset;
  }

private:
  size_t value = 0;
  size_t bitOffset = -8 * sizeof(size_t);
  const size_t *p;
  const size_t *end;

  void UpdateValue() {
    while (value == 0 && p < end) {
      value = *p++;
      bitOffset += 8 * sizeof(size_t);
    }
  }
};

template <size_t N> class BitField {
private:
  static constexpr size_t BITS_PER_WORD = 8 * sizeof(size_t);
  static constexpr size_t WORD_COUNT = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;

public:
  bool IsSet(size_t n) const {
    return (data[n / BITS_PER_WORD] & (1ULL << n % BITS_PER_WORD)) != 0;
  }

  bool IsAnySet() const {
    size_t v = data[0];
    for (size_t i = 1; i < WORD_COUNT; ++i) {
      v |= data[i];
    }
    return v != 0;
  }

  size_t PopCount() const {
    size_t result = 0;
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      result += Bit<sizeof(size_t)>::PopCount(data[i]);
    }
    return result;
  }

  // Returns the bitmask for the specified range [startIndex, endIndex).
  // Undefined results if it crosses a size_t boundary.
  size_t GetRange(size_t startIndex, size_t endIndex) const {
    const size_t value = data[startIndex / BITS_PER_WORD];
    const size_t topClearShift = BITS_PER_WORD - endIndex % BITS_PER_WORD;
    const size_t bottomClearShift = startIndex % BITS_PER_WORD;
    return value << topClearShift >> (bottomClearShift + topClearShift);
  }

  void Set(size_t n) { data[n / BITS_PER_WORD] |= 1ULL << n % BITS_PER_WORD; }
  void Clear(size_t n) {
    data[n / BITS_PER_WORD] &= ~(1ULL << n % BITS_PER_WORD);
  }

  void ClearAll() {
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      data[i] = 0;
    }
  }

  BitField operator~() const {
    BitField result;
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      result.data[i] = ~data[i];
    }
    return result;
  }

  BitField operator&(const BitField &other) const {
    BitField result;
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      result.data[i] = data[i] & other.data[i];
    }
    return result;
  }

  BitField operator|(const BitField &other) const {
    BitField result;
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      result.data[i] = data[i] | other.data[i];
    }
    return result;
  }

  BitField operator^(const BitField &other) const {
    BitField result;
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      result.data[i] = data[i] ^ other.data[i];
    }
    return result;
  }

  void operator&=(const BitField &other) {
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      data[i] &= other.data[i];
    }
  }

  void operator|=(const BitField &other) {
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      data[i] |= other.data[i];
    }
  }

  void operator^=(const BitField &other) {
    for (size_t i = 0; i < WORD_COUNT; ++i) {
      data[i] ^= other.data[i];
    }
  }

  bool operator==(const BitField &other) const {
    return Mem::Eq(data, other.data, sizeof(data));
  }

  friend BitFieldIterator begin(const BitField &b) {
    return BitFieldIterator(b.data, b.data + WORD_COUNT);
  }

  // Dummy response, as BitFieldIterator does not use this.
  friend const size_t end(const BitField &b) { return 0; }

  static constexpr size_t BIT_COUNT = N;

private:
  size_t data[WORD_COUNT];
};

//---------------------------------------------------------------------------

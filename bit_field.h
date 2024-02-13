//---------------------------------------------------------------------------

#pragma once
#include "bit.h"
#include <string.h>

//---------------------------------------------------------------------------

class BitFieldIterator {
public:
  BitFieldIterator(const size_t *p, const size_t *end) : p(p), end(end) {
    MoveNext();
  }

  bool operator!=(const size_t *end) const { return value != 0 || p != end; }

  void operator++() { MoveNext(); }

  size_t operator*() const {
    return Bit<sizeof(size_t)>::CountTrailingZeros(value) + bitOffset;
  }

private:
  size_t value = 0;
  size_t bitOffset = -8 * sizeof(size_t);
  const size_t *p;
  const size_t *end;

  void MoveNext() {
    // Pull off lowest bit.
    value = value & (value - 1);
    while (value == 0 && p < end) {
      value = *p++;
      bitOffset += 8 * sizeof(size_t);
    }
  }
};

template <size_t N> class BitField {
private:
  static const size_t BITS_PER_WORD = 8 * sizeof(size_t);
  static const size_t WORD_COUNT = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;

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
    size_t value = data[startIndex / BITS_PER_WORD];
    size_t topClearShift = BITS_PER_WORD - endIndex % BITS_PER_WORD;
    size_t bottomClearShift = startIndex % BITS_PER_WORD;
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
    return memcmp(data, other.data, sizeof(data)) == 0;
  }
  bool operator!=(const BitField &other) const {
    return memcmp(data, other.data, sizeof(data)) != 0;
  }

  friend BitFieldIterator begin(const BitField &b) {
    return BitFieldIterator(b.data, b.data + WORD_COUNT);
  }

  friend const size_t *end(const BitField &b) { return b.data + WORD_COUNT; }

  static const size_t BIT_COUNT = N;

private:
  size_t data[WORD_COUNT];
};

//---------------------------------------------------------------------------

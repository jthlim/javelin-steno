//---------------------------------------------------------------------------

#pragma once
#include "bit.h"
#include "container/fast_iterable.h"
#include "container/list.h"
#include "container/sized_list.h"
#include <assert.h>

//---------------------------------------------------------------------------

enum StrokeKeyType : uint8_t {
  MASK,
  SEPARATOR,
  SEPARATOR_FOLLOW_MASK,
};

struct StrokeKey {
  uint16_t c;
  StrokeKeyType type;
  uint32_t mask;

  bool IsSingleBit() const { return (mask & (mask - 1)) == 0; }
};

//---------------------------------------------------------------------------

struct StrokeBitIndex {
  enum {
    NUM,
    SL,
    TL,
    KL,
    PL,
    WL,
    HL,
    RL,
    A,
    O,
    STAR,
    E,
    U,
    FR,
    RR,
    PR,
    BR,
    LR,
    GR,
    TR,
    SR,
    DR,
    ZR,
    COUNT,

    UNICODE = 31,
    NONE = -1
  };
};

struct StrokeMask {
  enum {
    NUM = (uint32_t)1 << StrokeBitIndex::NUM,
    SL = (uint32_t)1 << StrokeBitIndex::SL,
    TL = (uint32_t)1 << StrokeBitIndex::TL,
    KL = (uint32_t)1 << StrokeBitIndex::KL,
    PL = (uint32_t)1 << StrokeBitIndex::PL,
    WL = (uint32_t)1 << StrokeBitIndex::WL,
    HL = (uint32_t)1 << StrokeBitIndex::HL,
    RL = (uint32_t)1 << StrokeBitIndex::RL,
    A = (uint32_t)1 << StrokeBitIndex::A,
    O = (uint32_t)1 << StrokeBitIndex::O,
    STAR = (uint32_t)1 << StrokeBitIndex::STAR,
    E = (uint32_t)1 << StrokeBitIndex::E,
    U = (uint32_t)1 << StrokeBitIndex::U,
    FR = (uint32_t)1 << StrokeBitIndex::FR,
    RR = (uint32_t)1 << StrokeBitIndex::RR,
    PR = (uint32_t)1 << StrokeBitIndex::PR,
    BR = (uint32_t)1 << StrokeBitIndex::BR,
    LR = (uint32_t)1 << StrokeBitIndex::LR,
    GR = (uint32_t)1 << StrokeBitIndex::GR,
    TR = (uint32_t)1 << StrokeBitIndex::TR,
    SR = (uint32_t)1 << StrokeBitIndex::SR,
    DR = (uint32_t)1 << StrokeBitIndex::DR,
    ZR = (uint32_t)1 << StrokeBitIndex::ZR,

    ALL = ((uint32_t)1 << StrokeBitIndex::COUNT) - 1,

    UNICODE = (uint32_t)1 << StrokeBitIndex::UNICODE,
  };
};

//---------------------------------------------------------------------------

// This represents all of the steno keys, where different keys (e.g. number,
// star, S) are represented by the *same* bits.
class StenoStroke {
public:
  StenoStroke() = default;

  constexpr StenoStroke(uint32_t keyState) : keyState(keyState) {}

  void Set(const char *string);
  template <size_t N> StenoStroke(const char (&s)[N]) { Set(s); }

  bool IsEmpty() const { return keyState == 0; }
  bool IsNotEmpty() const { return keyState != 0; }

  // Buffer must be at least MAX_STRING_LENGTH characters wide.
  char *ToString(char *buffer) const;

  bool operator==(const StenoStroke &o) const { return keyState == o.keyState; }

  StenoStroke operator&(const StenoStroke &o) const {
    return StenoStroke(keyState & o.keyState);
  }
  StenoStroke operator|(const StenoStroke &o) const {
    return StenoStroke(keyState | o.keyState);
  }
  StenoStroke operator^(const StenoStroke &o) const {
    return StenoStroke(keyState ^ o.keyState);
  }

  void operator&=(const StenoStroke &o) { keyState &= o.keyState; }
  void operator|=(const StenoStroke &o) { keyState |= o.keyState; }
  void operator^=(const StenoStroke &o) { keyState ^= o.keyState; }

  StenoStroke operator~() const { return StenoStroke(~keyState); }

  uint32_t GetKeyState() const { return keyState; }

  uint32_t PopCount() const {
    return Bit<sizeof(keyState)>::PopCount(keyState);
  }
  uint32_t Hash() const { return StenoStroke::Hash(this, 1); }

  static uint32_t PopCount(const StenoStroke *strokes, size_t length);
  static uint32_t Hash(const StenoStroke *strokes, size_t length);
  static bool Equals(const StenoStroke *a, const StenoStroke *b,
                     size_t length) {
    assert(length != 0);
    do {
      if (*a++ != *b++) {
        return false;
      }
    } while (--length);
    return true;
  }

  static bool HasPartialOutline(const StenoStroke *outline,
                                const StenoStroke *strokes, size_t length,
                                size_t maxOffset) {
    const StenoStroke firstStroke = strokes[0];
    for (size_t i = 0; i <= maxOffset; ++i) {
      if (outline[i] == firstStroke) [[unlikely]] {
        if (Equals(strokes, outline + i, length)) {
          return true;
        }
      }
    }
    return false;
  }

  void CopyTo(StenoStroke *destination, size_t length) const {
    for (size_t i = 0; i < length; ++i) {
      destination[i] = this[i];
    }
  }

  static void SetLanguage(const SizedList<StrokeKey> &keys);

  static constexpr size_t MAX_STRING_LENGTH = 94;

private:
  uint32_t keyState;

  static FastIterable<const StrokeKey> formatter;
  static FastIterable<const StrokeKey> parser;
  static List<StrokeKey> formatters;
  static List<StrokeKey> parsers;
};

static_assert(sizeof(StenoStroke) == 4);

//---------------------------------------------------------------------------

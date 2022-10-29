//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

struct ChordBitIndex {
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

    NONE = -1
  };
};

struct ChordMask {
  enum {
    NUM = (uint32_t)1 << ChordBitIndex::NUM,
    SL = (uint32_t)1 << ChordBitIndex::SL,
    TL = (uint32_t)1 << ChordBitIndex::TL,
    KL = (uint32_t)1 << ChordBitIndex::KL,
    PL = (uint32_t)1 << ChordBitIndex::PL,
    WL = (uint32_t)1 << ChordBitIndex::WL,
    HL = (uint32_t)1 << ChordBitIndex::HL,
    RL = (uint32_t)1 << ChordBitIndex::RL,
    A = (uint32_t)1 << ChordBitIndex::A,
    O = (uint32_t)1 << ChordBitIndex::O,
    STAR = (uint32_t)1 << ChordBitIndex::STAR,
    E = (uint32_t)1 << ChordBitIndex::E,
    U = (uint32_t)1 << ChordBitIndex::U,
    FR = (uint32_t)1 << ChordBitIndex::FR,
    RR = (uint32_t)1 << ChordBitIndex::RR,
    PR = (uint32_t)1 << ChordBitIndex::PR,
    BR = (uint32_t)1 << ChordBitIndex::BR,
    LR = (uint32_t)1 << ChordBitIndex::LR,
    GR = (uint32_t)1 << ChordBitIndex::GR,
    TR = (uint32_t)1 << ChordBitIndex::TR,
    SR = (uint32_t)1 << ChordBitIndex::SR,
    DR = (uint32_t)1 << ChordBitIndex::DR,
    ZR = (uint32_t)1 << ChordBitIndex::ZR,

    ALL = ((uint32_t)1 << ChordBitIndex::COUNT) - 1,
  };
};

//---------------------------------------------------------------------------

// This represents all of the steno keys, where different keys (e.g. number,
// star, S) are represented by the *same* bits.
class StenoChord {
public:
  constexpr StenoChord(uint32_t keyState = 0) : keyState(keyState) {}

  // Only for use in tests.
  StenoChord(const char *string);

  bool IsEmpty() const { return keyState == 0; }
  bool IsNotEmpty() const { return keyState != 0; }

  // Buffer must be at least 26 characters wide.
  size_t ToString(char *buffer) const;

  bool operator==(const StenoChord &o) const { return keyState == o.keyState; }
  bool operator!=(const StenoChord &o) const { return keyState != o.keyState; }

  StenoChord operator&(const StenoChord &o) const {
    return StenoChord(keyState & o.keyState);
  }
  StenoChord operator|(const StenoChord &o) const {
    return StenoChord(keyState | o.keyState);
  }
  void operator&=(const StenoChord &o) { keyState &= o.keyState; }
  StenoChord operator~() const { return StenoChord(~keyState); }

  uint32_t GetKeyState() const { return keyState; }

  uint32_t Hash() const { return StenoChord::Hash(this, 1); }

  static uint32_t Hash(const StenoChord *chords, size_t length);

private:
  uint32_t keyState;

  // Returns pointer to the first character after vowels, '*' or '-'.
  // Points to a null if it doesn't exist.
  static const char *RightStart(const char *p);
};

//---------------------------------------------------------------------------

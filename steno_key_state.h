//---------------------------------------------------------------------------

#pragma once
#include "stroke.h"

//---------------------------------------------------------------------------

enum class StenoKey : int8_t {
  S1,
  S2,
  TL,
  KL,
  PL,
  WL,
  HL,
  RL,
  A,
  O,
  STAR1,
  STAR2,
  STAR3,
  STAR4,
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
  NUM1,
  NUM2,
  NUM3,
  NUM4,
  NUM5,
  NUM6,
  NUM7,
  NUM8,
  NUM9,
  NUM10,
  NUM11,
  NUM12,
  FUNCTION, // X1, ^
  POWER,    // X2, +
  RES1,     // X3
  RES2,     // X4
  X1 = FUNCTION,
  X2,
  X3,
  X4,
  X5,
  X6,
  X7,
  X8,
  X9,
  X10,
  X11,
  X12,
  X13,
  X14,
  X15,
  X16,
  X17,
  X18,
  X19,
  X20,
  X21,
  X22,
  X23,
  X24,
  X25,
  X26,

  COUNT,

  NONE = -1
};

static_assert(int(StenoKey::COUNT) == 64, "Expect StenoKey to have 64 bits");

//---------------------------------------------------------------------------

// This represents all of the steno keys, where different keys (e.g. number,
// star, S) are represented by the *different* bits.
class StenoKeyState {
public:
  constexpr StenoKeyState(uint64_t keyState = 0) : keyState(keyState) {}

  void Process(StenoKey keycode, bool isPress);

  void Reset() { keyState = 0; }
  bool IsEmpty() const { return keyState == 0; }
  bool IsNotEmpty() const { return keyState != 0; }

  bool operator==(const StenoKeyState &other) const = default;

  StenoKeyState operator~() const { return StenoKeyState(~keyState); }
  StenoKeyState operator&(const StenoKeyState &other) const {
    return StenoKeyState(keyState & other.keyState);
  }
  StenoKeyState operator|(const StenoKeyState &other) const {
    return StenoKeyState(keyState | other.keyState);
  }

  void operator&=(const StenoKeyState &other) { keyState &= other.keyState; }
  void operator|=(const StenoKeyState &other) { keyState |= other.keyState; }

  StenoStroke ToStroke() const;
  uint64_t GetRawKeyState() const { return keyState; }

  // Public so that config can overwrite S1 -> NUM1.
  static int8_t STROKE_BIT_INDEX_LOOKUP[];

private:
  uint64_t keyState;
};

//---------------------------------------------------------------------------

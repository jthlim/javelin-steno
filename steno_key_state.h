//---------------------------------------------------------------------------

#pragma once
#include "stroke.h"

//---------------------------------------------------------------------------

struct StenoGeminiPacket {
  uint8_t data[6];
};

struct StenoPloverHidPacket {
  uint8_t data[8];
};

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
  FUNCTION,
  POWER,
  RES1,
  RES2,

  COUNT,

  NONE = -1
};

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

  bool operator==(const StenoKeyState &other) const {
    return keyState == other.keyState;
  }
  bool operator!=(const StenoKeyState &other) const {
    return keyState != other.keyState;
  }
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
  StenoGeminiPacket ToGeminiPacket() const;
  StenoPloverHidPacket ToPloverHidPacket() const;

  // Public so that config can overwrite S1 -> NUM1.
  static int8_t STROKE_BIT_INDEX_LOOKUP[];

private:
  uint64_t keyState;
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "steno_key_state.h"
#include <assert.h>

//---------------------------------------------------------------------------

#define USE_CTZLL 1

//---------------------------------------------------------------------------

int8_t StenoKeyState::STROKE_BIT_INDEX_LOOKUP[] = {
    StrokeBitIndex::SL,   // S1
    StrokeBitIndex::SL,   // S2
    StrokeBitIndex::TL,   // TL
    StrokeBitIndex::KL,   // KL
    StrokeBitIndex::PL,   // PL
    StrokeBitIndex::WL,   // WL
    StrokeBitIndex::HL,   // HL
    StrokeBitIndex::RL,   // RL
    StrokeBitIndex::A,    // A
    StrokeBitIndex::O,    // O
    StrokeBitIndex::STAR, // STAR1
    StrokeBitIndex::STAR, // STAR2
    StrokeBitIndex::STAR, // STAR3
    StrokeBitIndex::STAR, // STAR4
    StrokeBitIndex::E,    // E
    StrokeBitIndex::U,    // U
    StrokeBitIndex::FR,   // FR
    StrokeBitIndex::RR,   // RR
    StrokeBitIndex::PR,   // PR
    StrokeBitIndex::BR,   // BR
    StrokeBitIndex::LR,   // LR
    StrokeBitIndex::GR,   // GR
    StrokeBitIndex::TR,   // TR
    StrokeBitIndex::SR,   // SR
    StrokeBitIndex::DR,   // DR
    StrokeBitIndex::ZR,   // ZR
    StrokeBitIndex::NUM,  // NUM1
    StrokeBitIndex::NUM,  // NUM2
    StrokeBitIndex::NUM,  // NUM3
    StrokeBitIndex::NUM,  // NUM4
    StrokeBitIndex::NUM,  // NUM5
    StrokeBitIndex::NUM,  // NUM6
    StrokeBitIndex::NUM,  // NUM7
    StrokeBitIndex::NUM,  // NUM8
    StrokeBitIndex::NUM,  // NUM9
    StrokeBitIndex::NUM,  // NUM10
    StrokeBitIndex::NUM,  // NUM11
    StrokeBitIndex::NUM,  // NUM12
    StrokeBitIndex::NONE, // X1 / FUNCTION
    StrokeBitIndex::NONE, // X2 / POWER
    StrokeBitIndex::NONE, // X3 / RES1
    StrokeBitIndex::NONE, // X4 / RES2
    StrokeBitIndex::NONE, // X5
    StrokeBitIndex::NONE, // X6
    StrokeBitIndex::NONE, // X7
    StrokeBitIndex::NONE, // X8
    StrokeBitIndex::NONE, // X9
    StrokeBitIndex::NONE, // X10
    StrokeBitIndex::NONE, // X11
    StrokeBitIndex::NONE, // X12
    StrokeBitIndex::NONE, // X13
    StrokeBitIndex::NONE, // X14
    StrokeBitIndex::NONE, // X15
    StrokeBitIndex::NONE, // X16
    StrokeBitIndex::NONE, // X17
    StrokeBitIndex::NONE, // X18
    StrokeBitIndex::NONE, // X19
    StrokeBitIndex::NONE, // X20
    StrokeBitIndex::NONE, // X21
    StrokeBitIndex::NONE, // X22
    StrokeBitIndex::NONE, // X23
    StrokeBitIndex::NONE, // X24
    StrokeBitIndex::NONE, // X25
    StrokeBitIndex::NONE, // X26
};

static_assert(sizeof(StenoKeyState::STROKE_BIT_INDEX_LOOKUP) ==
                  (int)StenoKey::COUNT,
              "StrokeBitIndex table must be complete");

//---------------------------------------------------------------------------

void StenoKeyState::Process(StenoKey key, bool isPress) {
  if (isPress) {
    keyState |= 1ULL << (int)key;
  } else {
    keyState &= ~(1ULL << (int)key);
  }
}

StenoStroke StenoKeyState::ToStroke() const {
  uint32_t strokeKeyState = 0;

#if USE_CTZLL
  uint64_t localKeyState = keyState;
  while (localKeyState) {
    const int index = __builtin_ctzll(localKeyState);
    const int shift = STROKE_BIT_INDEX_LOOKUP[index];
    if (shift != -1) {
      strokeKeyState |= 1UL << shift;
    }

    // Zero the lowest bit.
    localKeyState &= localKeyState - 1;
  }
#else
  for (size_t i = 0; i < sizeof(STROKE_BIT_INDEX_LOOKUP); ++i) {
    if (keyState & (1ULL << i)) {
      int shift = STROKE_BIT_INDEX_LOOKUP[i];
      if (shift != -1) {
        strokeKeyState |= 1UL << shift;
      }
    }
  }
#endif

  return StenoStroke(strokeKeyState);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "str.h"
#include "unit_test.h"

TEST_BEGIN("StenoKeyState tests") {
  char buffer[StenoStroke::MAX_STRING_LENGTH];

  StenoKeyState state;
  state.Process(StenoKey::HL, true);
  state.Process(StenoKey::LR, true);
  {
    const StenoStroke stroke = state.ToStroke();
    stroke.ToString(buffer);
    assert(Str::Eq(buffer, "H-L"));
  }
}
TEST_END

//---------------------------------------------------------------------------

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
    StrokeBitIndex::NONE, // FUNCTION
    StrokeBitIndex::NONE, // POWER
    StrokeBitIndex::NONE, // RES1
    StrokeBitIndex::NONE, // RES2
};

static_assert(sizeof(StenoKeyState::STROKE_BIT_INDEX_LOOKUP) ==
                  (int)StenoKey::COUNT,
              "StrokeBitIndex table must be complete");

constexpr uint8_t GEMINI_LOOKUP[] = {
    14, // S1
    13, // S2
    12, // TL
    11, // KL
    10, // PL
    9,  // WL
    8,  // HL
    22, // RL
    21, // A
    20, // O
    19, // STAR1
    18, // STAR2
    29, // STAR3
    28, // STAR4
    27, // E
    26, // U
    25, // FR
    24, // RR
    38, // PR
    37, // BR
    36, // LR
    35, // GR
    34, // TR
    33, // SR
    32, // DR
    40, // ZR
    5,  // NUM1
    4,  // NUM2
    3,  // NUM3
    2,  // NUM4
    1,  // NUM5
    0,  // NUM6
    46, // NUM7
    45, // NUM8
    44, // NUM9
    43, // NUM10
    42, // NUM11
    41, // NUM12
    6,  // FUNCTION
    30, // POWER
    17, // RES1
    16, // RES2
};

static_assert(sizeof(GEMINI_LOOKUP) == (int)StenoKey::COUNT,
              "Gemini table must be complete");

constexpr uint8_t PLOVER_HID_LOOKUP[] = {
    7,  // S1
    7,  // S2
    6,  // TL
    5,  // KL
    4,  // PL
    3,  // WL
    2,  // HL
    1,  // RL
    0,  // A
    15, // O
    14, // STAR1
    14, // STAR2
    14, // STAR3
    14, // STAR4
    13, // E
    12, // U
    11, // FR
    10, // RR
    9,  // PR
    8,  // BR
    23, // LR
    22, // GR
    21, // TR
    20, // SR
    19, // DR
    18, // ZR
    17, // NUM1
    17, // NUM2
    17, // NUM3
    17, // NUM4
    17, // NUM5
    17, // NUM6
    17, // NUM7
    17, // NUM8
    17, // NUM9
    17, // NUM10
    17, // NUM11
    17, // NUM12
    16, // FUNCTION
    15, // POWER
    14, // RES1
    13, // RES2
};

static_assert(sizeof(PLOVER_HID_LOOKUP) == (int)StenoKey::COUNT,
              "Plover HID table must be complete");

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
    int index = __builtin_ctzll(localKeyState);
    int shift = STROKE_BIT_INDEX_LOOKUP[index];
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

StenoGeminiPacket StenoKeyState::ToGeminiPacket() const {
  StenoGeminiPacket result = {.data = {0x80, 0, 0, 0, 0, 0}};
  uint64_t localKeyState = keyState;
  while (localKeyState) {
    int index = __builtin_ctzll(localKeyState);
    uint8_t geminiIndex = GEMINI_LOOKUP[index];

    int byte = geminiIndex / 8;
    int offset = geminiIndex % 8;
    result.data[byte] |= 1 << offset;

    localKeyState &= localKeyState - 1;
  }
  return result;
}

StenoPloverHidPacket StenoKeyState::ToPloverHidPacket() const {
  StenoPloverHidPacket result = {};
  uint64_t localKeyState = keyState;
  while (localKeyState) {
    int index = __builtin_ctzll(localKeyState);
    uint8_t ploverHidIndex = PLOVER_HID_LOOKUP[index];

    int byte = ploverHidIndex / 8;
    int offset = ploverHidIndex % 8;
    result.data[byte] |= 1 << offset;

    localKeyState &= localKeyState - 1;
  }
  return result;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "unit_test.h"
#include <string.h>

TEST_BEGIN("StenoKeyState tests") {
  char buffer[StenoStroke::MAX_STRING_LENGTH];

  StenoKeyState state;
  state.Process(StenoKey::HL, true);
  state.Process(StenoKey::LR, true);
  {
    const StenoStroke stroke = state.ToStroke();
    stroke.ToString(buffer);
    assert(strcmp(buffer, "H-L") == 0);
  }
}
TEST_END

//---------------------------------------------------------------------------

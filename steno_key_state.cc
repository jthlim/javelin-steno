//---------------------------------------------------------------------------

#include "steno_key_state.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

#define USE_CTZLL 1

//---------------------------------------------------------------------------

int8_t StenoKeyState::CHORD_BIT_INDEX_LOOKUP[] = {
    ChordBitIndex::SL,   // S1
    ChordBitIndex::SL,   // S2
    ChordBitIndex::TL,   // TL
    ChordBitIndex::KL,   // KL
    ChordBitIndex::PL,   // PL
    ChordBitIndex::WL,   // WL
    ChordBitIndex::HL,   // HL
    ChordBitIndex::RL,   // RL
    ChordBitIndex::A,    // A
    ChordBitIndex::O,    // O
    ChordBitIndex::STAR, // STAR1
    ChordBitIndex::STAR, // STAR2
    ChordBitIndex::STAR, // STAR3
    ChordBitIndex::STAR, // STAR4
    ChordBitIndex::E,    // E
    ChordBitIndex::U,    // U
    ChordBitIndex::FR,   // FR
    ChordBitIndex::RR,   // RR
    ChordBitIndex::PR,   // PR
    ChordBitIndex::BR,   // BR
    ChordBitIndex::LR,   // LR
    ChordBitIndex::GR,   // GR
    ChordBitIndex::TR,   // TR
    ChordBitIndex::SR,   // SR
    ChordBitIndex::DR,   // DR
    ChordBitIndex::ZR,   // ZR
    ChordBitIndex::NUM,  // NUM1
    ChordBitIndex::NUM,  // NUM2
    ChordBitIndex::NUM,  // NUM3
    ChordBitIndex::NUM,  // NUM4
    ChordBitIndex::NUM,  // NUM5
    ChordBitIndex::NUM,  // NUM6
    ChordBitIndex::NUM,  // NUM7
    ChordBitIndex::NUM,  // NUM8
    ChordBitIndex::NUM,  // NUM9
    ChordBitIndex::NUM,  // NUM10
    ChordBitIndex::NUM,  // NUM11
    ChordBitIndex::NUM,  // NUM12
    ChordBitIndex::NONE, // FUNCTION
    ChordBitIndex::NONE, // POWER
    ChordBitIndex::NONE, // RES1
    ChordBitIndex::NONE, // RES2
};

static_assert(sizeof(StenoKeyState::CHORD_BIT_INDEX_LOOKUP) ==
                  (int)StenoKey::COUNT,
              "ChordBitIndex table must be complete");

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

StenoChord StenoKeyState::ToChord() const {
  uint32_t chordKeyState = 0;

#if USE_CTZLL
  uint64_t localKeyState = keyState;
  while (localKeyState) {
    int index = __builtin_ctzll(localKeyState);
    int shift = CHORD_BIT_INDEX_LOOKUP[index];
    if (shift != -1) {
      chordKeyState |= 1UL << shift;
    }

    // Zero the lowest bit.
    localKeyState &= localKeyState - 1;
  }
#else
  for (size_t i = 0; i < sizeof(CHORD_BIT_INDEX_LOOKUP); ++i) {
    if (keyState & (1ULL << i)) {
      int shift = CHORD_BIT_INDEX_LOOKUP[i];
      if (shift != -1) {
        chordKeyState |= 1UL << shift;
      }
    }
  }
#endif

  return StenoChord(chordKeyState);
}

StenoGeminiPacket StenoKeyState::ToGeminiPacket() const {
  StenoGeminiPacket result = {.data = {0x80, 0, 0, 0, 0, 0}};
  uint64_t localKeyState = keyState;
  while (localKeyState) {
    int index = __builtin_ctzll(localKeyState);
    unsigned int geminiIndex = GEMINI_LOOKUP[index];

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
    unsigned int ploverHidIndex = PLOVER_HID_LOOKUP[index];

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

TEST_BEGIN("StenoKeyState tests") {
  char buffer[32];

  StenoKeyState state;
  state.Process(StenoKey::HL, true);
  state.Process(StenoKey::LR, true);
  {
    const StenoChord chord = state.ToChord();
    chord.ToString(buffer);
    assert(strcmp(buffer, "H-L") == 0);
  }
}
TEST_END

//---------------------------------------------------------------------------

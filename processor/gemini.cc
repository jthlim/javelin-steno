//---------------------------------------------------------------------------

#include "gemini.h"
#include "../console.h"
#include "../hal/serial_port.h"

//---------------------------------------------------------------------------

constexpr uint8_t INVALID_GEMINI_KEY = 0xff;

constexpr uint8_t GEMINI_LOOKUP[] = {
    14,                 // S1
    13,                 // S2
    12,                 // TL
    11,                 // KL
    10,                 // PL
    9,                  // WL
    8,                  // HL
    22,                 // RL
    21,                 // A
    20,                 // O
    19,                 // STAR1
    18,                 // STAR2
    29,                 // STAR3
    28,                 // STAR4
    27,                 // E
    26,                 // U
    25,                 // FR
    24,                 // RR
    38,                 // PR
    37,                 // BR
    36,                 // LR
    35,                 // GR
    34,                 // TR
    33,                 // SR
    32,                 // DR
    40,                 // ZR
    5,                  // NUM1
    4,                  // NUM2
    3,                  // NUM3
    2,                  // NUM4
    1,                  // NUM5
    0,                  // NUM6
    46,                 // NUM7
    45,                 // NUM8
    44,                 // NUM9
    43,                 // NUM10
    42,                 // NUM11
    41,                 // NUM12
    6,                  // FUNCTION, X1, ^
    30,                 // POWER, X2, +
    17,                 // RES1, X3
    16,                 // RES2, X4
    INVALID_GEMINI_KEY, // X5
    INVALID_GEMINI_KEY, // X6
    INVALID_GEMINI_KEY, // X7
    INVALID_GEMINI_KEY, // X8
    INVALID_GEMINI_KEY, // X9
    INVALID_GEMINI_KEY, // X10
    INVALID_GEMINI_KEY, // X11
    INVALID_GEMINI_KEY, // X12
    INVALID_GEMINI_KEY, // X13
    INVALID_GEMINI_KEY, // X14
    INVALID_GEMINI_KEY, // X15
    INVALID_GEMINI_KEY, // X16
    INVALID_GEMINI_KEY, // X17
    INVALID_GEMINI_KEY, // X18
    INVALID_GEMINI_KEY, // X19
    INVALID_GEMINI_KEY, // X20
    INVALID_GEMINI_KEY, // X21
    INVALID_GEMINI_KEY, // X22
    INVALID_GEMINI_KEY, // X23
    INVALID_GEMINI_KEY, // X24
    INVALID_GEMINI_KEY, // X25
    INVALID_GEMINI_KEY, // X26
};
static_assert(sizeof(GEMINI_LOOKUP) == (int)StenoKey::COUNT,
              "Gemini table must be complete");

struct StenoGeminiPacket {
public:
  StenoGeminiPacket(const StenoKeyState &state);

  uint8_t data[6];
};

StenoGeminiPacket::StenoGeminiPacket(const StenoKeyState &state)
    : data{0x80, 0, 0, 0, 0, 0} {
  uint64_t localKeyState = state.GetRawKeyState();
  while (localKeyState) {
    const int index = __builtin_ctzll(localKeyState);
    const uint8_t geminiIndex = GEMINI_LOOKUP[index];
    if (geminiIndex != INVALID_GEMINI_KEY) {
      const int byte = geminiIndex / 8;
      const int offset = geminiIndex % 8;
      data[byte] |= 1 << offset;
    }

    localKeyState &= localKeyState - 1;
  }
}

//---------------------------------------------------------------------------

void StenoGemini::Process(const StenoKeyState &value, StenoAction action) {
  next->Process(value, action);
  if (action != StenoAction::TRIGGER) {
    return;
  }

  ++counter;
  const StenoGeminiPacket packet(value);
  SerialPort::SendData(packet.data, sizeof(packet.data));
}

void StenoGemini::PrintInfo() const {
  Console::Printf("  Gemini: %zu strokes\n", counter);
}

//---------------------------------------------------------------------------

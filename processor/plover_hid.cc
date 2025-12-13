//---------------------------------------------------------------------------

#include "plover_hid.h"
#include "../console.h"

//---------------------------------------------------------------------------

constexpr uint8_t PLOVER_HID_LOOKUP[] = {
    7,  // S1
    16, // S2
    6,  // TL
    5,  // KL
    4,  // PL
    3,  // WL
    2,  // HL
    1,  // RL
    0,  // A
    15, // O
    14, // STAR1
    31, // STAR2
    30, // STAR3
    29, // STAR4
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
    28, // NUM2
    27, // NUM3
    26, // NUM4
    25, // NUM5
    24, // NUM6
    39, // NUM7
    38, // NUM8
    37, // NUM9
    36, // NUM10
    35, // NUM11
    34, // NUM12
    33, // FUNCTION, ^ -> X1
    32, // POWER, + -> X2
    47, // RES1 -> X3
    46, // RES2 -> X4
    45, // X5
    44, // X6
    43, // X7
    42, // X8
    41, // X9
    40, // X10
    55, // X11
    54, // X12
    53, // X13
    52, // X14
    51, // X15
    50, // X16
    49, // X17
    48, // X18
    63, // X19
    62, // X20
    61, // X21
    60, // X22
    59, // X23
    58, // X24
    57, // X25
    56, // X26
};

static_assert(sizeof(PLOVER_HID_LOOKUP) == (int)StenoKey::COUNT,
              "Plover HID table must be complete");

//---------------------------------------------------------------------------

StenoPloverHidPacket::StenoPloverHidPacket(const StenoKeyState &state)
    : data{} {
  uint64_t localKeyState = state.GetRawKeyState();
  while (localKeyState) {
    const int index = __builtin_ctzll(localKeyState);
    const uint8_t ploverHidIndex = PLOVER_HID_LOOKUP[index];

    const int byte = ploverHidIndex / 8;
    const int offset = ploverHidIndex % 8;
    data[byte] |= 1 << offset;

    localKeyState &= localKeyState - 1;
  }
}

//---------------------------------------------------------------------------

void StenoPloverHid::Process(const StenoKeyState &value, StenoAction action) {
  next->Process(value, action);
  switch (action) {
  case StenoAction::CANCEL_KEY:
  case StenoAction::CANCEL_ALL:
  case StenoAction::TRIGGER:
    break;

  case StenoAction::PRESS:
  case StenoAction::RELEASE:
    ++counter;
    const StenoPloverHidPacket packet(value);
    SendPacket(packet);
  }
}

void StenoPloverHid::PrintInfo() const {
  Console::Printf("  Plover HID: %zu packets\n", counter);
}

//---------------------------------------------------------------------------

[[gnu::weak]] void
StenoPloverHid::SendPacket(const StenoPloverHidPacket &packet) {}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include "plover_hid.h"
#include "../console.h"

//---------------------------------------------------------------------------

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
    31, // POWER
    30, // RES1
    29, // RES2
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

__attribute__((weak)) void
StenoPloverHid::SendPacket(const StenoPloverHidPacket &packet) {}

//---------------------------------------------------------------------------

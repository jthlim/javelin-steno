//---------------------------------------------------------------------------

#include "tx_bolt.h"
#include "../console.h"
#include "../serial_port.h"

//---------------------------------------------------------------------------

struct StenoTxBoltPacket {
  uint8_t length = 0;
  uint8_t data[5] = {};

  void Set(const StenoStroke &stroke);
};

struct TxBoltCode {
  uint8_t group;
  uint8_t bitmask;
}

constexpr TX_BOLT_LOOKUP[] = {
    {3, 0b00010000}, // NUM
    {0, 0b00000001}, // SL
    {0, 0b00000010}, // TL
    {0, 0b00000100}, // KL
    {0, 0b00001000}, // PL
    {0, 0b00010000}, // WL
    {0, 0b00100000}, // HL
    {1, 0b00000001}, // RL
    {1, 0b00000010}, // A
    {1, 0b00000100}, // O
    {1, 0b00001000}, // STAR
    {1, 0b00010000}, // E
    {1, 0b00100000}, // U
    {2, 0b00000001}, // FR
    {2, 0b00000010}, // RR
    {2, 0b00000100}, // PR
    {2, 0b00001000}, // BR
    {2, 0b00010000}, // LR
    {2, 0b00100000}, // GR
    {3, 0b00000001}, // TR
    {3, 0b00000010}, // SR
    {3, 0b00000100}, // DR
    {3, 0b00001000}, // ZR
};
static_assert(sizeof(TX_BOLT_LOOKUP) / sizeof(TxBoltCode) ==
                  StrokeBitIndex::COUNT,
              "TX bolt table must be complete");

void StenoTxBoltPacket::Set(const StenoStroke &stroke) {
  // The TX bolt packet is split into groups:
  //
  //  Group 0: SL, TL, KL, PL, WL, HL
  //  Group 1: RL, A, O, STAR, E, U
  //  Group 2: FR, RR, PR, BR, LR, GR
  //  Group 3: TR, SR, DR, ZR, NUM
  //
  // Each group is encoded as top two bits for the group id, then a bitfield.
  // If all bits in a group are 0, then the entire byte should be skipped.

  uint32_t localKeyState = stroke.GetKeyState();
  while (localKeyState) {
    int index = __builtin_ctzl(localKeyState);
    const TxBoltCode boltCode = TX_BOLT_LOOKUP[index];
    data[boltCode.group] |= boltCode.bitmask;

    localKeyState &= localKeyState - 1;
  }

  for (size_t i = 0; i < 4; ++i) {
    uint8_t bits = data[i];
    if (bits) {
      data[length++] = (i << 6) | bits;
    }
  }
  data[length++] = 0;
};

//---------------------------------------------------------------------------

void StenoTxBolt::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }

  ++counter;

  StenoTxBoltPacket packet;
  packet.Set(value.ToStroke());
  SerialPort::SendData(packet.data, packet.length);
}

void StenoTxBolt::PrintInfo() const {
  Console::Printf("  TX Bolt: %zu strokes\n", counter);
}

//---------------------------------------------------------------------------

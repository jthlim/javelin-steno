//---------------------------------------------------------------------------

#include "infrared.h"

//---------------------------------------------------------------------------

[[gnu::weak]] void
Infrared::SendData(uint64_t data, int bitCount,
                   const InfraredDataConfiguration &configuration) {
  // Do nothing.
}

//---------------------------------------------------------------------------

void Infrared::SendMessage(InfraredProtocol protocol, uint32_t d0, uint32_t d1,
                           uint32_t d2) {
  switch (protocol) {
  case InfraredProtocol::RC5:
    SendRC5Message(d0, d1, d2);
    break;
  case InfraredProtocol::NEC:
    SendNECMessage(d0, d1);
    break;
  }
}

void Infrared::SendNECMessage(uint32_t address, uint32_t command) {
  static constexpr InfraredDataConfiguration configuration = {
      .carrierFrequency = 38000,
      .dutyCycle = 33,
      .loopCount = 21,
      .header = {16, 8},
      .zeroBit = {1, 1},
      .oneBit = {1, 3},
      .trailer = {1, 0},
  };

  uint64_t message = ((address & 0xff) << 24) | ((~address & 0xff) << 16) |
                     ((command & 0xff) << 8) | (~command & 0xff);
  SendData(message << 32, 32, configuration);
}

// Extra is the toggle bit.
void Infrared::SendRC5Message(uint32_t address, uint32_t command,
                              uint32_t toggle) {
  union RC5Message {
    uint32_t value;
    struct {
      uint32_t command : 6;
      uint32_t address : 5;
      uint32_t toggle : 1;
      uint32_t commandHighNot : 1;
      uint32_t start : 1;
    };
  };

  const RC5Message message = {
      .command = command,
      .address = address,
      .toggle = toggle,
      .commandHighNot = ~(command >> 6),
      .start = 1,
  };

  const uint32_t value = message.value;

  // Manchester encode it.
  uint64_t data = 0;
  for (size_t i = 0; i < 14; ++i) {
    data <<= 2;
    if (value & (1 << (13 - i))) {
      data |= 0b01;
    } else {
      data |= 0b10;
    }
  }

  static constexpr InfraredDataConfiguration configuration = {
      .carrierFrequency = 36000,
      .dutyCycle = 25,
      .loopCount = 32,
      .header = {0, 0},
      .zeroBit = {0, 1},
      .oneBit = {1, 0},
      .trailer = {0, 0},
  };
  SendData(data << 36, 28, configuration);
}

//---------------------------------------------------------------------------

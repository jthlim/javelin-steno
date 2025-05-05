//---------------------------------------------------------------------------

#include "infrared.h"
#include "../bit.h"

//---------------------------------------------------------------------------

static constexpr InfraredDataConfiguration SONY_CONFIGURATION = {
    .carrierFrequency = 40000,
    .dutyCycle = 33,
    .cycleCount = 24,
    .header = {4, 1},
    .zeroBit = {1, 1},
    .oneBit = {2, 1},
    .trailer = {0, 0},
};

//---------------------------------------------------------------------------

#if RUN_TESTS
static uint64_t infraRedData;
static int infraredBitCount;
[[gnu::weak]] void
Infrared::SendData(uint64_t data, int bitCount,
                   const InfraredDataConfiguration &configuration) {
  infraRedData = data;
  infraredBitCount = bitCount;
}

#else
[[gnu::weak]] void
Infrared::SendData(uint64_t data, int bitCount,
                   const InfraredDataConfiguration &configuration) {}
#endif

[[gnu::weak]] void Infrared::Stop() {}

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
  case InfraredProtocol::SAMSUNG:
    SendSamsungMessage(d0, d1);
    break;
  case InfraredProtocol::SONY12:
    SendSony12Message(d0, d1);
    break;
  case InfraredProtocol::SONY15:
    SendSony15Message(d0, d1);
    break;
  case InfraredProtocol::SONY20:
    SendSony20Message(d0, d1, d2);
    break;
  }
}

void Infrared::SendNECMessage(uint32_t address, uint32_t command) {
  address = Bit<1>::ReverseBits(address);
  command = Bit<1>::ReverseBits(command);

  static constexpr InfraredDataConfiguration configuration = {
      .carrierFrequency = 38000,
      .dutyCycle = 33,
      .cycleCount = 21,
      .header = {16, 8},
      .zeroBit = {1, 1},
      .oneBit = {1, 3},
      .trailer = {1, 0},
  };

  const uint64_t message = (address << 24) | ((~address & 0xff) << 16) |
                           (command << 8) | (~command & 0xff);
  SendData(message << 32, 32, configuration);
}

// Address is 5 bits.
// Command is 6 bits.
// Toggle is 1 bit.
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

  uint32_t value = message.value << 18;

  // Manchester encode it.
  uint32_t data = 0;
  for (size_t i = 0; i < 14; ++i) {
    data = (data << 2) | (2 >> (value >> 31));
    value <<= 1;
  }

  static constexpr InfraredDataConfiguration configuration = {
      .carrierFrequency = 36000,
      .dutyCycle = 25,
      .cycleCount = 32,
      .header = {0, 0},
      .zeroBit = {0, 1},
      .oneBit = {1, 0},
      .trailer = {0, 0},
  };
  SendData((uint64_t)data << 36, 28, configuration);
}

void Infrared::SendSamsungMessage(uint32_t address, uint32_t command) {
  address = Bit<1>::ReverseBits(address);
  command = Bit<1>::ReverseBits(command);

  static constexpr InfraredDataConfiguration configuration = {
      .carrierFrequency = 38000,
      .dutyCycle = 33,
      .cycleCount = 21,
      .header = {8, 8},
      .zeroBit = {1, 1},
      .oneBit = {1, 3},
      .trailer = {1, 1},
  };

  const uint64_t message =
      (address << 24) | (address << 16) | (command << 8) | (~command & 0xff);
  SendData(message << 32, 32, configuration);
}

void Infrared::SendSony12Message(uint32_t address, uint32_t command) {
  address = Bit<1>::ReverseBits(address);
  command = Bit<1>::ReverseBits(command);

  const uint64_t message = (command << 4) | (address >> 3);
  SendData(message << 52, 12, SONY_CONFIGURATION);
}

void Infrared::SendSony15Message(uint32_t address, uint32_t command) {
  address = Bit<1>::ReverseBits(address);
  command = Bit<1>::ReverseBits(command);

  const uint64_t message = (command << 7) | address;
  SendData(message << 49, 15, SONY_CONFIGURATION);
}

void Infrared::SendSony20Message(uint32_t address, uint32_t command,
                                 uint32_t extended) {
  address = Bit<1>::ReverseBits(address);
  command = Bit<1>::ReverseBits(command);
  extended = Bit<1>::ReverseBits(extended);

  const uint64_t message = (command << 12) | (address << 5) | extended;
  SendData(message << 44, 20, SONY_CONFIGURATION);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

TEST_BEGIN("Infrared NEC data is calculated correctly") {
  Infrared::SendMessage(InfraredProtocol::NEC, 0, 0xad, 0);
  assert(
      infraRedData ==
      0b00000000'11111111'10110101'01001010'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared RC5 data is calculated correctly") {
  Infrared::SendMessage(InfraredProtocol::RC5, 0x5, 0x35, 0);
  assert(
      infraRedData ==
      0b0101'10'1010011001'010110011001'0000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 28);

  Infrared::SendMessage(InfraredProtocol::RC5, 0x5, 0x75, 1);
  assert(
      infraRedData ==
      0b0110'01'1010011001'010110011001'0000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 28);
}
TEST_END

TEST_BEGIN("Infrared Samsung data is calculated correctly") {
  Infrared::SendMessage(InfraredProtocol::SAMSUNG, 0x7, 0x4, 0);
  assert(
      infraRedData ==
      0b11100000'11100000'00100000'11011111'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared Sony12 data is calculated correctly") {
  Infrared::SendMessage(InfraredProtocol::SONY12, 1, 0x13, 0);
  assert(
      infraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 12);
}
TEST_END

TEST_BEGIN("Infrared Sony15 data is calculated correctly") {
  Infrared::SendMessage(InfraredProtocol::SONY15, 1, 0x13, 0);
  assert(
      infraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 15);
}
TEST_END

TEST_BEGIN("Infrared Sony20 data is calculated correctly") {
  Infrared::SendMessage(InfraredProtocol::SONY20, 1, 0x13, 0x30);
  assert(
      infraRedData ==
      0b1100100'10000'00001100'0000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 20);
}
TEST_END

//---------------------------------------------------------------------------

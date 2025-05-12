//---------------------------------------------------------------------------

#include "infrared.h"
#include "../bit.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

#if RUN_TESTS
static uint64_t infraRedData;
static int infraredBitCount;
[[gnu::weak]] void
Infrared::SendData(const void *data, int bitCount,
                   const InfraredDataConfiguration &configuration) {
  infraredBitCount = bitCount;
  infraRedData = 0;
  const uint8_t *p = (const uint8_t *)data;
  uint32_t mask = 0;
  for (size_t i = 0; i < bitCount; ++i) {
    if ((mask << 1) == 0) {
      mask = (*p++ << 24) | 0x800000;
    }
    if (mask >> 31) {
      infraRedData |= 1ull << (63 - i);
    }
    mask <<= 1;
  }
}

#else
[[gnu::weak]] void
Infrared::SendData(const void *data, int bitCount,
                   const InfraredDataConfiguration &configuration) {}
#endif

[[gnu::weak]] void Infrared::Stop() {}

//---------------------------------------------------------------------------

void Infrared::SendMessage(const char *protocolName, uint32_t d0, uint32_t d1,
                           uint32_t d2) {
  struct InfraredProtocol {
    const char *name;
    void (*handler)(uint32_t, uint32_t, uint32_t);
  };

  static constexpr InfraredProtocol protocols[] = {
      {"dyson", SendDysonMessage},     //
      {"nec", SendNECMessage},         //
      {"rca", SendRCAMessage},         //
      {"rc5", SendRC5Message},         //
      {"samsung", SendSamsungMessage}, //
      {"sony38", SendSony38Message},   //
      {"sony40", SendSony40Message},   //
  };
  for (const InfraredProtocol &protocol : protocols) {
    if (Str::Eq(protocolName, protocol.name)) {
      protocol.handler(d0, d1, d2);
      return;
    }
  }

  Console::Printf("Unknown infrared protocol: %s\n", protocolName);
  Console::Printf("Protocol list:\n");
  for (const InfraredProtocol &protocol : protocols) {
    Console::Printf("- %s\n", protocol.name);
  }
  Console::Print("\n");
}

void Infrared::SendDysonMessage(uint32_t address, uint32_t command,
                                uint32_t _) {
  address = Bit<4>::ReverseBits(address);
  command = Bit<4>::ReverseBits(command);

  static constexpr InfraredDataConfiguration configuration = {
      .playbackCount = 2,
      .repeatDelay = 100000,
      .repeatDelayMode = InfraredRepeatDelayMode::END_TO_START,
      .carrierFrequency = 38000,
      .dutyCycle = 33,
      .cycleCount = 30, // 789µs.
      .header = {3, 1},
      .zeroBit = {1, 1},
      .oneBit = {1, 2},
      .trailer = {1, 0},
  };

  const uint32_t data = address | (command >> 7);
  uint8_t message[2];
  message[0] = data >> 24;
  message[1] = data >> 16;
  SendData(message, 15, configuration);
}

void Infrared::SendNECMessage(uint32_t address, uint32_t command, uint32_t _) {
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

  uint8_t message[4];
  message[0] = address;
  message[1] = ~address;
  message[2] = command;
  message[3] = ~command;
  SendData(message, 32, configuration);
}

void Infrared::SendRCAMessage(uint32_t address, uint32_t command, uint32_t _) {
  static constexpr InfraredDataConfiguration configuration = {
      .carrierFrequency = 56000,
      .dutyCycle = 33,
      .cycleCount = 28,
      .header = {8, 8},
      .zeroBit = {1, 2},
      .oneBit = {1, 4},
      .trailer = {0, 0},
  };

  const uint32_t combined = address << 8 | command;
  const uint32_t message = (combined << 12) | (~combined & 0xfff);
  uint8_t data[3];
  data[0] = message >> 16;
  data[1] = message >> 8;
  data[2] = message;
  SendData(data, 24, configuration);
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

  uint8_t dataBytes[4];
  dataBytes[0] = data >> 20;
  dataBytes[1] = data >> 12;
  dataBytes[2] = data >> 4;
  dataBytes[3] = data << 4;
  SendData(dataBytes, 28, configuration);
}

void Infrared::SendSamsungMessage(uint32_t address, uint32_t command,
                                  uint32_t _) {
  address = Bit<1>::ReverseBits(address);
  command = Bit<1>::ReverseBits(command);

  static constexpr InfraredDataConfiguration configuration = {
      .playbackCount = (uint32_t)-1,
      .repeatDelay = 108300,
      .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
      .carrierFrequency = 38000,
      .dutyCycle = 33,
      .cycleCount = 21,
      .header = {8, 8},
      .zeroBit = {1, 1},
      .oneBit = {1, 3},
      .trailer = {1, 1},
  };

  uint8_t message[4];
  message[0] = address;
  message[1] = address;
  message[2] = command;
  message[3] = ~command;
  SendData(message, 32, configuration);
}

void Infrared::SendSony38Message(uint32_t address, uint32_t command,
                                 uint32_t bits) {
  static constexpr InfraredDataConfiguration configuration = {
      .playbackCount = (uint32_t)-1,
      .repeatDelay = 45000,
      .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
      .carrierFrequency = 38000,
      .dutyCycle = 33,
      .cycleCount = 23,
      .header = {4, 1},
      .zeroBit = {1, 1},
      .oneBit = {2, 1},
      .trailer = {0, 0},
  };
  SendSonyMessage(address, command, bits, configuration);
}

void Infrared::SendSony40Message(uint32_t address, uint32_t command,
                                 uint32_t bits) {
  static constexpr InfraredDataConfiguration configuration = {
      .playbackCount = (uint32_t)-1,
      .repeatDelay = 45000,
      .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
      .carrierFrequency = 40000,
      .dutyCycle = 33,
      .cycleCount = 24,
      .header = {4, 1},
      .zeroBit = {1, 1},
      .oneBit = {2, 1},
      .trailer = {0, 0},
  };
  SendSonyMessage(address, command, bits, configuration);
}

void Infrared::SendSonyMessage(uint32_t address, uint32_t command,
                               uint32_t bits,
                               const InfraredDataConfiguration &configuration) {
  switch (bits) {
  case 0:
    // Try and use the lowest number of bits.
    if (command < 0x80) {
      if (address < 0x20) {
        return SendSony12Message(address, command, configuration);
      } else if (address < 0x100) {
        return SendSony15Message(address, command, configuration);
      }
    } else if (command < 0x8000 && address < 0x20) {
      return SendSony20Message(address, command, configuration);
    }
    Console::Printf("Unable to encode message\n\n");
    return;
  case 12:
    return SendSony12Message(address, command, configuration);

  case 15:
    return SendSony15Message(address, command, configuration);

  case 20:
    return SendSony20Message(address, command, configuration);

  default:
    Console::Printf(
        "Sony protocol requires 0(infer)/12/15/20 bits as last parameter\n\n");
  }
}

void Infrared::SendSony12Message(
    uint32_t address, uint32_t command,
    const InfraredDataConfiguration &configuration) {
  address = Bit<4>::ReverseBits(address);
  command = Bit<4>::ReverseBits(command);

  const uint32_t data = command | (address >> 7);
  uint8_t message[2];
  message[0] = data >> 24;
  message[1] = data >> 16;
  SendData(message, 12, configuration);
}

void Infrared::SendSony15Message(
    uint32_t address, uint32_t command,
    const InfraredDataConfiguration &configuration) {
  address = Bit<4>::ReverseBits(address);
  command = Bit<4>::ReverseBits(command);

  const uint32_t data = command | (address >> 7);
  uint8_t message[2];
  message[0] = data >> 24;
  message[1] = data >> 16;
  SendData(message, 15, configuration);
}

// Command is 15 bits.
void Infrared::SendSony20Message(
    uint32_t address, uint32_t command,
    const InfraredDataConfiguration &configuration) {
  const uint32_t extended = Bit<4>::ReverseBits(command >> 7);
  address = Bit<4>::ReverseBits(address);
  command = Bit<4>::ReverseBits(command & 0x7f);

  const uint32_t data = command | (address >> 7) | (extended >> 12);

  uint8_t message[3];
  message[0] = data >> 24;
  message[1] = data >> 16;
  message[2] = data >> 8;
  SendData(message, 20, configuration);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "../unit_test.h"

TEST_BEGIN("Infrared Dyson data is calculated correctly") {
  Infrared::SendMessage("Dyson", 9, 0x3f, 0);
  assert(
      infraRedData ==
      0b1001000'11111100'0'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 15);
}
TEST_END

TEST_BEGIN("Infrared NEC data is calculated correctly") {
  Infrared::SendMessage("NEC", 0, 0xad, 0);
  assert(
      infraRedData ==
      0b00000000'11111111'10110101'01001010'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared RCA data is calculated correctly") {
  Infrared::SendMessage("RCA", 10, 0x68, 0);
  assert(
      infraRedData ==
      0b1010'01101000'0101'10010111'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 24);
}
TEST_END

TEST_BEGIN("Infrared RC5 data is calculated correctly") {
  Infrared::SendMessage("RC5", 0x5, 0x35, 0);
  assert(
      infraRedData ==
      0b0101'10'1010011001'010110011001'0000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 28);

  Infrared::SendMessage("RC5", 0x5, 0x75, 1);
  assert(
      infraRedData ==
      0b0110'01'1010011001'010110011001'0000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 28);
}
TEST_END

TEST_BEGIN("Infrared Samsung data is calculated correctly") {
  Infrared::SendMessage("Samsung", 0x7, 0x4, 0);
  assert(
      infraRedData ==
      0b11100000'11100000'00100000'11011111'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared Sony12 data is calculated correctly") {
  Infrared::SendMessage("Sony", 1, 0x13, 12);
  assert(
      infraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 12);
}
TEST_END

TEST_BEGIN("Infrared Sony15 data is calculated correctly") {
  Infrared::SendMessage("Sony", 1, 0x13, 15);
  assert(
      infraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 15);
}
TEST_END

TEST_BEGIN("Infrared Sony20 data is calculated correctly") {
  Infrared::SendMessage("Sony", 1, 0x13 + (0x39 << 7), 20);
  assert(
      infraRedData ==
      0b1100100'10000'10011100'0000'00000000'00000000'00000000'00000000'00000000ull);
  assert(infraredBitCount == 20);
}
TEST_END

//---------------------------------------------------------------------------

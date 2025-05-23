//---------------------------------------------------------------------------

#include "infrared.h"
#include "../bit.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

struct RawInfraredData {
  size_t index = 0;
  FixedPoint<uint16_t, 2> data[512];
  FixedPoint<uint32_t, 2> totalTime;

  void Add(const InfraredDataConfiguration::PulseTime &time) {
    if (index >= 510) {
      return;
    }
    totalTime.value += time.onTime.value;
    totalTime.value += time.offTime.value;

    if (time.onTime != 0) {
      if (index & 1) {
        // Last time was on, extend it
        data[index - 1] += time.onTime;
      } else {
        data[index++] = time.onTime;
      }
    }

    if (time.offTime != 0) {
      if (index & 1) {
        data[index++] = time.offTime;
      } else {
        data[index - 1] += time.offTime;
      }
    }
  }

  void Trim() {
    // If last time is for an off-pulse, remove it.
    if ((index & 1) == 0) {
      --index;
    }
  }

  bool IsValid() const { return index > 0 && index < 510; }
};

//---------------------------------------------------------------------------

#if RUN_TESTS
static uint64_t testInfraRedData;
static int testInfraredBitCount;
void SetInfraredDataBits(const void *data, int bitCount) {
  testInfraredBitCount = bitCount;
  testInfraRedData = 0;
  const uint8_t *p = (const uint8_t *)data;
  uint32_t mask = 0;
  for (size_t i = 0; i < bitCount; ++i) {
    if ((mask << 1) == 0) {
      mask = (*p++ << 24) | 0x800000;
    }
    if (mask >> 31) {
      testInfraRedData |= 1ull << (63 - i);
    }
    mask <<= 1;
  }
}

FixedPoint<uint16_t, 2> testRawData[512];
size_t testRawDataCount;

void Infrared::SendRawData(const FixedPoint<uint16_t, 2> *data,
                           size_t dataCount,
                           const InfraredRawDataConfiguration &configuration) {
  memcpy(testRawData, data, 2 * dataCount);
  testRawDataCount = dataCount;
}
#else
[[gnu::weak]] void
Infrared::SendRawData(const FixedPoint<uint16_t, 2> *data, size_t dataCount,
                      const InfraredRawDataConfiguration &configuration) {}
#endif

[[gnu::weak]] void Infrared::Stop() {}

//---------------------------------------------------------------------------

void Infrared::SendData(const void *data, size_t bitCount,
                        const InfraredDataConfiguration &configuration) {
  if (bitCount > 250) {
    Console::Printf("ERR Infrared data too long\n\n");
    return;
  }

#if RUN_TESTS
  SetInfraredDataBits(data, bitCount);
#endif

  RawInfraredData rawData;
  rawData.Add(configuration.header);

  const uint8_t *p = (const uint8_t *)data;
  for (size_t i = 0; i < bitCount; ++i) {
    rawData.Add(configuration.GetBitPulseTime((p[i / 8] >> (~i & 7)) & 1));
  }
  rawData.Add(configuration.trailer);

  if (rawData.IsValid()) {
    rawData.Trim();
    SendRawData(rawData.data, rawData.index, configuration.rawConfiguration);
  }
}

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
      {"sirc", SendSircMessage},       //
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

  constexpr float TICK = 789.5;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 2,
              .carrierFrequency = 38000,
              .dutyCycle = 33,
              .repeatDelay = 100000,
              .repeatDelayMode = InfraredRepeatDelayMode::END_TO_START,
          },
      .header = {3 * TICK, 1 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 2 * TICK},
      .trailer = {1 * TICK, 0},
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

  constexpr float TICK = 552.63f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .carrierFrequency = 38000,
              .dutyCycle = 33,
          },
      .header = {16 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0},
  };

  uint8_t message[4];
  message[0] = address;
  message[1] = ~address;
  message[2] = command;
  message[3] = ~command;
  SendData(message, 32, configuration);
}

void Infrared::SendRCAMessage(uint32_t address, uint32_t command, uint32_t _) {
  constexpr float TICK = 500;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .carrierFrequency = 56000,
              .dutyCycle = 33,
          },
      .header = {8 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 2 * TICK},
      .oneBit = {1 * TICK, 4 * TICK},
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

  constexpr float TICK = 888.89f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .carrierFrequency = 36000,
              .dutyCycle = 25,
          },
      .header = {0, 0},
      .zeroBit = {0, TICK},
      .oneBit = {TICK, 0},
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

  constexpr float TICK = 552.63f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .carrierFrequency = 38000,
              .dutyCycle = 33,
              .repeatDelay = 108300,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
          },
      .header = {8 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0 * TICK},
  };

  uint8_t message[4];
  message[0] = address;
  message[1] = address;
  message[2] = command;
  message[3] = ~command;
  SendData(message, 32, configuration);
}

void Infrared::SendSircMessage(uint32_t address, uint32_t command,
                               uint32_t bits) {
  constexpr float TICK = 600;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .carrierFrequency = 40000,
              .dutyCycle = 33,
              .repeatDelay = 45000,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
          },
      .header = {4 * TICK, 1 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {2 * TICK, 1 * TICK},
      .trailer = {0, 0},
  };

  switch (bits) {
  case 0:
    // Try and use the lowest number of bits.
    if (command < 0x80) {
      if (address < 0x20) {
        return SendSirc12Message(address, command, configuration);
      } else if (address < 0x100) {
        return SendSirc15Message(address, command, configuration);
      }
    } else if (command < 0x8000 && address < 0x20) {
      return SendSirc20Message(address, command, configuration);
    }
    Console::Printf("Unable to encode message\n\n");
    return;
  case 12:
    return SendSirc12Message(address, command, configuration);

  case 15:
    return SendSirc15Message(address, command, configuration);

  case 20:
    return SendSirc20Message(address, command, configuration);

  default:
    Console::Printf("Sirc protocol requires 0(infer)/12/15/20 bits as "
                    "last parameter\n\n");
  }
}

void Infrared::SendSirc12Message(
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

void Infrared::SendSirc15Message(
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
void Infrared::SendSirc20Message(
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
  Infrared::SendMessage("dyson", 9, 0x3f, 0);
  assert(
      testInfraRedData ==
      0b1001000'11111100'0'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 15);
}
TEST_END

TEST_BEGIN("Infrared NEC data is calculated correctly") {
  Infrared::SendMessage("nec", 0, 0xad, 0);
  assert(
      testInfraRedData ==
      0b00000000'11111111'10110101'01001010'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared RCA data is calculated correctly") {
  Infrared::SendMessage("rca", 10, 0x68, 0);
  assert(
      testInfraRedData ==
      0b1010'01101000'0101'10010111'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 24);
}
TEST_END

TEST_BEGIN("Infrared RC5 data is calculated correctly") {
  Infrared::SendMessage("rc5", 0x5, 0x35, 0);
  assert(
      testInfraRedData ==
      0b0101'10'1010011001'010110011001'0000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 28);

  Infrared::SendMessage("rc5", 0x5, 0x75, 1);
  assert(
      testInfraRedData ==
      0b0110'01'1010011001'010110011001'0000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 28);
}
TEST_END

TEST_BEGIN("Infrared Samsung data is calculated correctly") {
  Infrared::SendMessage("samsung", 0x7, 0x4, 0);
  assert(
      testInfraRedData ==
      0b11100000'11100000'00100000'11011111'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared Sirc12 data is calculated correctly") {
  Infrared::SendMessage("sirc", 1, 0x13, 12);
  assert(
      testInfraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 12);

  assert(testRawDataSize == 25);
  // Header
  assert(testRawData[0] == 4 * 600);
  assert(testRawData[1] == 1 * 600);
  // Data
  assert(testRawData[2] == 2 * 600);
  assert(testRawData[3] == 1 * 600);
  assert(testRawData[4] == 2 * 600);
  assert(testRawData[5] == 1 * 600);
  assert(testRawData[6] == 1 * 600);
  assert(testRawData[7] == 1 * 600);
}
TEST_END

TEST_BEGIN("Infrared Sirc15 data is calculated correctly") {
  Infrared::SendMessage("sirc", 1, 0x13, 15);
  assert(
      testInfraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 15);
}
TEST_END

TEST_BEGIN("Infrared Sirc20 data is calculated correctly") {
  Infrared::SendMessage("sirc", 1, 0x13 + (0x39 << 7), 20);
  assert(
      testInfraRedData ==
      0b1100100'10000'10011100'0000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 20);
}
TEST_END

//---------------------------------------------------------------------------

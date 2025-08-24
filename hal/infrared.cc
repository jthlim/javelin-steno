//---------------------------------------------------------------------------

#include "infrared.h"
#include "../console.h"
#include "../str.h"

//---------------------------------------------------------------------------

#if RUN_TESTS
static uint64_t testInfraRedData;
static size_t testInfraredBitCount;
void SetInfraredDataBits(const void *data, size_t bitCount,
                         InfraredEndianness endianness) {
  testInfraredBitCount = bitCount;
  testInfraRedData = 0;
  const uint8_t *p = (const uint8_t *)data;
  uint32_t mask = 0;
  for (size_t i = 0; i < bitCount; ++i) {
    const uint32_t byte = p[i / 8];
    const size_t rightShift =
        endianness == InfraredEndianness::MSB_FIRST ? ~i : i;
    const uint32_t bit = (byte >> (rightShift & 7)) & 1;
    if (bit) {
      testInfraRedData |= 1ull << (63 - i);
    }
    mask <<= 1;
  }
}

InfraredTime testRawData[512];
size_t testRawDataCount;

void Infrared::SendRawData(const InfraredTime *data, size_t dataCount,
                           const InfraredRawDataConfiguration &configuration) {
  memcpy(testRawData, data, 2 * dataCount);
  testRawDataCount = dataCount;
}
#else
[[gnu::weak]] void
Infrared::SendRawData(const InfraredTime *data, size_t dataCount,
                      const InfraredRawDataConfiguration &configuration) {}
#endif

[[gnu::weak]] void Infrared::Stop() {}

//---------------------------------------------------------------------------

struct RawInfraredData {
  static constexpr size_t DATA_COUNT = 512;

  size_t index = 0;
  InfraredTime totalTime;
  InfraredTime data[DATA_COUNT];

  void AddOnTime(InfraredTime time);
  void AddOffTime(InfraredTime time);
  void Add(const InfraredDataConfiguration::PulseTime &time);

  void AddData(const void *data, size_t bitCount,
               const InfraredDataConfiguration &configuration);

  void Trim() {
    // If last time is for an off-pulse, remove it.
    if ((index & 1) == 0) {
      --index;
      totalTime -= data[index];
    }
  }

  bool IsValid() const { return index > 0 && index < DATA_COUNT - 2; }
};

void RawInfraredData::AddOnTime(InfraredTime time) {
  if (time == 0) {
    return;
  }
  if (index & 1) {
    // Last time was on, extend it
    data[index - 1] += time;
  } else {
    data[index++] = time;
  }
}

void RawInfraredData::AddOffTime(InfraredTime time) {
  if (time == 0) {
    return;
  }
  if (index & 1) {
    data[index++] = time;
  } else {
    // Last time was off, extend it
    data[index - 1] += time;
  }
}

void RawInfraredData::Add(const InfraredDataConfiguration::PulseTime &time) {
  if (index >= DATA_COUNT - 2) {
    return;
  }

  totalTime.value += time.onTime.value & 0x7fff;
  totalTime.value += time.offTime.value & 0x7fff;

  if (time.onTime.value & 0x8000) {
    AddOffTime(time.offTime);

    const InfraredTime onTime =
        InfraredTime(time.onTime.value & 0x7fff, InfraredTime::raw);
    AddOnTime(onTime);
  } else {
    AddOnTime(time.onTime);
    AddOffTime(time.offTime);
  }
}

void RawInfraredData::AddData(const void *data, size_t bitCount,
                              const InfraredDataConfiguration &configuration) {
  Add(configuration.header);

#if RUN_TESTS
  SetInfraredDataBits(data, bitCount,
                      configuration.rawConfiguration.endianness);
#endif
  const uint8_t *p = (const uint8_t *)data;
  for (size_t i = 0; i < bitCount; ++i) {
    const uint32_t byte = p[i / 8];
    const size_t rightShift = configuration.rawConfiguration.endianness ==
                                      InfraredEndianness::MSB_FIRST
                                  ? ~i
                                  : i;
    const uint32_t bit = (byte >> (rightShift & 7)) & 1;
    Add(configuration.GetBitPulseTime(bit));
  }
  Add(configuration.trailer[0]);
  Add(configuration.trailer[1]);
}

//---------------------------------------------------------------------------

void Infrared::SendData(const void *data, size_t bitCount,
                        const InfraredDataConfiguration &configuration) {
  if (bitCount > 250) {
    Console::Printf("ERR Infrared data too long\n\n");
    return;
  }

#if RUN_TESTS
  SetInfraredDataBits(data, bitCount,
                      configuration.rawConfiguration.endianness);
#endif

  RawInfraredData rawData;
  rawData.AddData(data, bitCount, configuration);

  if (rawData.IsValid()) {
    rawData.Trim();
    SendRawData(rawData.data, rawData.index, configuration.rawConfiguration);
  }
}

//---------------------------------------------------------------------------

void Infrared::SendMessage(const char *protocolName, uint32_t address,
                           uint32_t command, uint32_t extraData) {
  struct InfraredProtocol {
    const char *name;
    void (*handler)(uint32_t, uint32_t, uint32_t);
  };

  static constexpr InfraredProtocol protocols[] = {
      {"dyson", SendDysonMessage},       //
      {"jvc", SendJvcMessage},           //
      {"kaseikyo", SendKaseikyoMessage}, //
      {"nec", SendNECMessage},           //
      {"necx", SendNECXMessage},         //
      {"nec42", SendNEC42Message},       //
      {"nec42x", SendNEC42XMessage},     //
      {"rc5", SendRC5Message},           //
      {"rc6", SendRC6Message},           //
      {"rca", SendRCAMessage},           //
      {"samsung", SendSamsungMessage},   //
      {"sirc", SendSircMessage},         //
  };
  for (const InfraredProtocol &protocol : protocols) {
    if (Str::Eq(protocolName, protocol.name)) {
      protocol.handler(address, command, extraData);
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
  const uint32_t data = address | (command << 7);
  uint8_t message[2];
  message[0] = data;
  message[1] = data >> 8;

  constexpr float TICK = 789.5;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 2,
              .repeatDelayMode = InfraredRepeatDelayMode::END_TO_START,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 100000,
          },
      .header = {3 * TICK, 1 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 2 * TICK},
      .trailer = {1 * TICK, 0},
  };
  SendData(message, 15, configuration);
}

void Infrared::SendJvcMessage(uint32_t address, uint32_t command, uint32_t _) {
  uint8_t message[2];
  message[0] = address;
  message[1] = command;

  constexpr float TICK = 526.0f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .repeatDataMode = InfraredRepeatDataMode::NO_HEADER,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 55000,
          },
      .header = {16 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0},
  };
  SendData(message, 16, configuration);
}

void Infrared::SendKaseikyoMessage(uint32_t address, uint32_t command,
                                   uint32_t vendor) {
  const uint32_t vendorParityBytes = (vendor ^ (vendor >> 8));
  const uint32_t vendorParity =
      (vendorParityBytes ^ (vendorParityBytes >> 4)) & 0xf;

  uint8_t message[6];
  message[0] = vendor;
  message[1] = vendor >> 8;
  message[2] = vendorParity | (address << 4);
  message[3] = address >> 4;
  message[4] = command;
  message[5] = message[2] ^ message[3] ^ message[4];

  constexpr float TICK = 432.0f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .carrierFrequency = 37000,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 130000,
          },
      .header = {8 * TICK, 4 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0},
  };
  SendData(message, 48, configuration);
}

void Infrared::SendNECMessage(uint32_t address, uint32_t command, uint32_t _) {
  uint8_t message[4];
  message[0] = address;
  message[1] = ~address;
  message[2] = command;
  message[3] = ~command;

  SendNECData(message);
}

void Infrared::SendNECXMessage(uint32_t address, uint32_t command, uint32_t _) {
  uint8_t message[4];
  message[0] = address;
  message[1] = address >> 8;
  message[2] = command;
  message[3] = ~command;

  SendNECData(message);
}

void Infrared::SendNECData(const uint8_t *data) {
  constexpr float TICK = 562.5f;
  static constexpr InfraredDataConfiguration dataConfiguration = {
      .rawConfiguration =
          {
              .endianness = InfraredEndianness::LSB_FIRST,
          },
      .header = {16 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0},
  };

  static constexpr InfraredDataConfiguration repeatConfiguration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .repeatDataMode = InfraredRepeatDataMode::HEADER_AND_TRAILER,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 108000,
          },
      .header = {16 * TICK, 4 * TICK},
      .zeroBit = {0 * TICK, 0 * TICK},
      .oneBit = {0 * TICK, 0 * TICK},
      .trailer = {1 * TICK, 0},
  };

  RawInfraredData rawData;
  rawData.AddData(data, 32, dataConfiguration);
  if (rawData.IsValid()) {
    rawData.Trim();
    SendRawData(rawData.data, rawData.index,
                repeatConfiguration.rawConfiguration);
  }
}

void Infrared::SendNEC42Message(uint32_t address, uint32_t command,
                                uint32_t _) {
  const uint32_t addressData = (address & 0x1fff) | ((~address & 0x1fff) << 13);
  const uint32_t commandData = (command & 0xff) | ((~command & 0xff) << 8);

  const uint32_t upper24 = (addressData >> 24) | (commandData << 2);

  uint8_t message[6];
  message[0] = addressData;
  message[1] = addressData >> 8;
  message[2] = addressData >> 16;
  message[3] = upper24;
  message[4] = upper24 >> 8;
  message[5] = upper24 >> 16;

  SendNEC42Data(message);
}

void Infrared::SendNEC42XMessage(uint32_t address, uint32_t command,
                                 uint32_t _) {
  const uint32_t upper24 = (address >> 24) | (command << 2);

  uint8_t message[6];
  message[0] = address;
  message[1] = address >> 8;
  message[2] = address >> 16;
  message[3] = upper24;
  message[4] = upper24 >> 8;
  message[5] = upper24 >> 16;

  SendNEC42Data(message);
}

void Infrared::SendNEC42Data(const uint8_t *data) {
  constexpr float TICK = 562.5f;
  static constexpr InfraredDataConfiguration dataConfiguration = {
      .rawConfiguration =
          {
              .endianness = InfraredEndianness::LSB_FIRST,
          },
      .header = {16 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0},
  };

  static constexpr InfraredDataConfiguration repeatConfiguration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .repeatDataMode = InfraredRepeatDataMode::HEADER_AND_TRAILER,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 108000,
          },
      .header = {16 * TICK, 4 * TICK},
      .zeroBit = {0 * TICK, 0 * TICK},
      .oneBit = {0 * TICK, 0 * TICK},
      .trailer = {1 * TICK, 0},
  };

  RawInfraredData rawData;
  rawData.AddData(data, 42, dataConfiguration);
  if (rawData.IsValid()) {
    rawData.Trim();
    SendRawData(rawData.data, rawData.index,
                repeatConfiguration.rawConfiguration);
  }
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

  uint8_t dataBytes[2];
  dataBytes[0] = message.value >> 6;
  dataBytes[1] = message.value << 2;

  constexpr float TICK = 888.89f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .carrierFrequency = 36000,
              .dutyCycle = 25,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .repeatDelay = 114000,
          },
      .header = {0, 0},
      .zeroBit = {TICK, TICK},
      .oneBit =
          {
              InfraredTime(InfraredTime(TICK).value | 0x8000,
                           InfraredTime::raw),
              TICK,
          },
      .trailer = {0, 0},
  };
  SendData(dataBytes, 14, configuration);
}

void Infrared::SendRC6Message(uint32_t address, uint32_t command,
                              uint32_t header) {
  constexpr float TICK = 444.5f;

  RawInfraredData data;
  constexpr InfraredDataConfiguration::PulseTime HEADER = {6 * TICK, 2 * TICK};
  data.Add(HEADER);

  const uint32_t value =
      ((0x3 << 20) | (header << 16) | (address << 8) | command) << 10;
  for (size_t i = 0; i < 22; ++i) {
    const InfraredTime tick =
        (i == 3) ? InfraredTime(2 * TICK) : InfraredTime(TICK);
    if ((value << i) & 0x80000000) {
      data.AddOnTime(tick);
      data.AddOffTime(tick);
    } else {
      data.AddOffTime(tick);
      data.AddOnTime(tick);
    }
  }

  // data.AddOffTime(6 * TICK);

  static constexpr InfraredRawDataConfiguration configuration = {
      .playbackCount = 0,
      .carrierFrequency = 36000,
      .dutyCycle = 25,
      .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
      .repeatDelay = 100000,
  };
  SendRawData(data.data, data.index, configuration);
}

void Infrared::SendRCAMessage(uint32_t address, uint32_t command, uint32_t _) {
  const uint32_t combined = address << 8 | command;
  const uint32_t message = (combined << 12) | (~combined & 0xfff);
  uint8_t data[3];
  data[0] = message >> 16;
  data[1] = message >> 8;
  data[2] = message;

  constexpr float TICK = 500;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .carrierFrequency = 56000,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .repeatDelay = 64000,
          },
      .header = {8 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 2 * TICK},
      .oneBit = {1 * TICK, 4 * TICK},
      .trailer = {0, 0},
  };
  SendData(data, 24, configuration);
}

void Infrared::SendSamsungMessage(uint32_t address, uint32_t command,
                                  uint32_t _) {
  uint8_t message[4];
  message[0] = address;
  message[1] = address;
  message[2] = command;
  message[3] = ~command;

  constexpr float TICK = 552.63f;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 108300,
          },
      .header = {8 * TICK, 8 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {1 * TICK, 3 * TICK},
      .trailer = {1 * TICK, 0 * TICK},
  };
  SendData(message, 32, configuration);
}

void Infrared::SendSircMessage(uint32_t address, uint32_t command,
                               uint32_t bits) {
  if (command >= 0x80) {
    return Console::Printf("sirc protocol requires command between 0-127\n\n");
  }

  constexpr float TICK = 600;
  static constexpr InfraredDataConfiguration configuration = {
      .rawConfiguration =
          {
              .playbackCount = 0,
              .carrierFrequency = 40000,
              .dutyCycle = 33,
              .repeatDelayMode = InfraredRepeatDelayMode::START_TO_START,
              .endianness = InfraredEndianness::LSB_FIRST,
              .repeatDelay = 45000,
          },
      .header = {4 * TICK, 1 * TICK},
      .zeroBit = {1 * TICK, 1 * TICK},
      .oneBit = {2 * TICK, 1 * TICK},
      .trailer = {0, 0},
  };

  switch (bits) {
  case 0:
    // Try and use the lowest number of bits.
    if (address < 0x20) {
      return SendSircMessageBits(address, command, 12, configuration);
    } else if (address < 0x100) {
      return SendSircMessageBits(address, command, 15, configuration);
    } else if (address < 0x2000) {
      return SendSircMessageBits(address, command, 20, configuration);
    }
    return Console::Printf("sirc address must be between 0-8191\n\n");
  case 12:
    return SendSircMessageBits(address, command, 12, configuration);

  case 15:
    return SendSircMessageBits(address, command, 15, configuration);

  case 20:
    return SendSircMessageBits(address, command, 20, configuration);

  default:
    Console::Printf("sirc protocol requires 0(infer)/12/15/20 bits as "
                    "last parameter\n\n");
  }
}

[[gnu::noinline]]
void Infrared::SendSircMessageBits(
    uint32_t address, uint32_t command, uint32_t bits,
    const InfraredDataConfiguration &configuration) {
  const uint32_t data = command | (address << 7);

  uint8_t message[4];
  message[0] = data;
  message[1] = data >> 8;
  message[2] = data >> 16;

  // Writing an extra byte allows the compiler to optimize this sequence
  // to a single 32-bit write.
  message[3] = data >> 24;
  SendData(message, bits, configuration);
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

TEST_BEGIN("Infrared JVC data is calculated correctly") {
  Infrared::SendMessage("jvc", 0x12, 0xad, 0);
  assert(
      testInfraRedData ==
      0b01001000'10110101'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 16);
}
TEST_END

TEST_BEGIN("Infrared Kaseikyo data is calculated correctly") {
  Infrared::SendMessage("kaseikyo", 0x123, 0xad, 0x2002);
  assert(
      testInfraRedData ==
      0b01000000'00000100'0000'1100'01001000'10110101'11110001'00000000'00000000ull);
  assert(testInfraredBitCount == 48);
}
TEST_END

TEST_BEGIN("Infrared NEC data is calculated correctly") {
  Infrared::SendMessage("nec", 0x12, 0xad, 0);
  assert(
      testInfraRedData ==
      0b01001000'10110111'10110101'01001010'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared NECX data is calculated correctly") {
  Infrared::SendMessage("necx", 0x1234, 0xad, 0);
  assert(
      testInfraRedData ==
      0b00101100'01001000'10110101'01001010'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 32);
}
TEST_END

TEST_BEGIN("Infrared NEC42 data is calculated correctly") {
  Infrared::SendMessage("nec42", 0x123, 0xad, 0);
  assert(
      testInfraRedData ==
      0b1100010010000'0011101101111'10110101'01001010'000000'00000000'00000000ull);
  assert(testInfraredBitCount == 42);
}
TEST_END

TEST_BEGIN("Infrared NEC42X data is calculated correctly") {
  Infrared::SendMessage("nec42x", 0x123456, 0xad, 0);
  assert(
      testInfraRedData ==
      0b01101010'00101100'01001000'00'10110101'00000000'000000'00000000'00000000ull);
  assert(testInfraredBitCount == 42);
}
TEST_END

TEST_BEGIN("Infrared RC5 data is calculated correctly") {
  Infrared::SendMessage("rc5", 0x5, 0x35, 0);
  assert(
      testInfraRedData ==
      0b11'0'00101'110101'00'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 14);

  Infrared::SendMessage("rc5", 0x5, 0x75, 1);
  assert(
      testInfraRedData ==
      0b10'1'00101'110101'00'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 14);
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

  assert(testRawDataCount == 25);
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

  Infrared::SendMessage("sirc", 1, 0x13, 0);
  assert(
      testInfraRedData ==
      0b1100100'10000'0000'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 12);
}
TEST_END

TEST_BEGIN("Infrared Sirc15 data is calculated correctly") {
  Infrared::SendMessage("sirc", 0xa5, 0x13, 15);
  assert(
      testInfraRedData ==
      0b1100100'10100101'0'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 15);

  Infrared::SendMessage("sirc", 0xa5, 0x13, 0);
  assert(
      testInfraRedData ==
      0b1100100'10100101'0'00000000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 15);
}
TEST_END

TEST_BEGIN("Infrared Sirc20 data is calculated correctly") {
  Infrared::SendMessage("sirc", 0x15a5, 0x13, 20);
  assert(
      testInfraRedData ==
      0b1100100'1010010110101'0000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 20);

  Infrared::SendMessage("sirc", 0x15a5, 0x13, 0);
  assert(
      testInfraRedData ==
      0b1100100'1010010110101'0000'00000000'00000000'00000000'00000000'00000000ull);
  assert(testInfraredBitCount == 20);
}
TEST_END

//---------------------------------------------------------------------------

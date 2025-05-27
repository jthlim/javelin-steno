//---------------------------------------------------------------------------

#pragma once
#include "../fixed_point.h"
#include "../uint32.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum class InfraredRepeatDelayMode : uint16_t {
  START_TO_START,
  END_TO_START,
};

enum class InfraredRepeatDataMode : uint16_t {
  FULL,
  NO_HEADER,
};

enum class InfraredEndianness : uint16_t {
  MSB_FIRST,
  LSB_FIRST,
};

using InfraredTime = FixedPoint<uint16_t, 1>;

struct InfraredRawDataConfiguration {
  // Number of times to send the message.
  // Use 0 for infinite.
  uint16_t playbackCount = 1;

  uint16_t carrierFrequency = 38000;
  uint16_t dutyCycle = 33;

  // Delay after data, measured in microseconds.
  InfraredRepeatDelayMode repeatDelayMode : 1;
  InfraredRepeatDataMode repeatDataMode : 2;
  uint16_t filler : 12;
  InfraredEndianness endianness : 1;

  FixedPoint<Uint32, 1> repeatDelay;
};
static_assert(sizeof(InfraredRawDataConfiguration) == 12);

struct InfraredDataConfiguration {
  InfraredRawDataConfiguration rawConfiguration;
  // Time in microseconds.
  struct PulseTime {
    InfraredTime onTime;
    InfraredTime offTime;
  };
  PulseTime header;
  PulseTime zeroBit;
  PulseTime oneBit;
  PulseTime trailer;

  const PulseTime &GetBitPulseTime(size_t index) const {
    return (&zeroBit)[index];
  }
};

class Infrared {
public:
  static void SendData(const void *data, size_t bitCount,
                       const InfraredDataConfiguration &configuration);

  // playCount = 0 -> infinite playback.
  static void SendRawData(const InfraredTime *data, size_t dataCount,
                          const InfraredRawDataConfiguration &rawData);

  static void SendMessage(const char *protocolName, uint32_t d0, uint32_t d1,
                          uint32_t d2);

  static void Stop();

private:
  static void SendDysonMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendJvcMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendKaseikyoMessage(uint32_t address, uint32_t command,
                                  uint32_t vendor);
  static void SendNECMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendNECXMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendRC5Message(uint32_t address, uint32_t command,
                             uint32_t toggle);
  static void SendRC6Message(uint32_t address, uint32_t command,
                             uint32_t toggle);
  static void SendRCAMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendSamsungMessage(uint32_t address, uint32_t command,
                                 uint32_t _);
  static void SendSircMessage(uint32_t address, uint32_t command,
                              uint32_t bits);
  static void
  SendSircMessageBits(uint32_t address, uint32_t command, uint32_t bits,
                      const InfraredDataConfiguration &configuration);
};

//---------------------------------------------------------------------------

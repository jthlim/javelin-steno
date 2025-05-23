//---------------------------------------------------------------------------

#pragma once
#include "../fixed_point.h"
#include "../uint32.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

enum InfraredRepeatDelayMode : uint8_t {
  START_TO_START,
  END_TO_START,
};

struct InfraredRawDataConfiguration {
  // Number of times to send the message.
  // Use 0 for infinite.
  Uint32 playbackCount = 1;

  uint16_t carrierFrequency;
  uint16_t dutyCycle;

  // Delay after data, measured in microseconds.
  FixedPoint<Uint32, 2> repeatDelay;
  InfraredRepeatDelayMode repeatDelayMode;
};

struct InfraredDataConfiguration {
  InfraredRawDataConfiguration rawConfiguration;
  // Time in microseconds.
  struct PulseTime {
    FixedPoint<uint16_t, 2> onTime;
    FixedPoint<uint16_t, 2> offTime;
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
  static void SendRawData(const FixedPoint<uint16_t, 2> *data, size_t dataCount,
                          const InfraredRawDataConfiguration &rawData);

  static void SendMessage(const char *protocolName, uint32_t d0, uint32_t d1,
                          uint32_t d2);

  static void Stop();

private:
  static void SendDysonMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendNECMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendRCAMessage(uint32_t address, uint32_t command, uint32_t _);
  static void SendRC5Message(uint32_t address, uint32_t command,
                             uint32_t toggle);
  static void SendSamsungMessage(uint32_t address, uint32_t command,
                                 uint32_t _);
  static void SendSircMessage(uint32_t address, uint32_t command,
                              uint32_t bits);
  static void SendSirc12Message(uint32_t address, uint32_t command,
                                const InfraredDataConfiguration &configuration);
  static void SendSirc15Message(uint32_t address, uint32_t command,
                                const InfraredDataConfiguration &configuration);
  static void SendSirc20Message(uint32_t address, uint32_t command,
                                const InfraredDataConfiguration &configuration);
};

//---------------------------------------------------------------------------

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

struct InfraredDataConfiguration {
  // Number of times to send the message.
  // Use 0 for infinite.
  Uint32 playbackCount = 1;
  // Delay after data, measured in microseconds.
  Uint32 repeatDelay;
  InfraredRepeatDelayMode repeatDelayMode;

  uint16_t carrierFrequency;
  uint16_t dutyCycle;
  uint16_t cycleCount;

  // Time in multiples of period defined by carrierFrequency
  struct PulseTicks {
    FixedPoint<uint16_t, 8> onTicks;
    FixedPoint<uint16_t, 8> offTicks;
  };
  PulseTicks header;
  PulseTicks zeroBit;
  PulseTicks oneBit;
  PulseTicks trailer;

  const PulseTicks &GetBitPulseTicks(size_t index) const {
    return (&zeroBit)[index];
  }
};

class Infrared {
public:
  static void SendData(const void *data, int bitCount,
                       const InfraredDataConfiguration &configuration);

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
  static void SendSony38Message(uint32_t address, uint32_t command,
                                uint32_t bits);
  static void SendSony40Message(uint32_t address, uint32_t command,
                                uint32_t bits);
  static void SendSonyMessage(uint32_t address, uint32_t command, uint32_t bits,
                              const InfraredDataConfiguration &configuration);
  static void SendSony12Message(uint32_t address, uint32_t command,
                                const InfraredDataConfiguration &configuration);
  static void SendSony15Message(uint32_t address, uint32_t command,
                                const InfraredDataConfiguration &configuration);
  static void SendSony20Message(uint32_t address, uint32_t command,
                                const InfraredDataConfiguration &configuration);
};

//---------------------------------------------------------------------------

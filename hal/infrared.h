//---------------------------------------------------------------------------

#pragma once
#include "../fixed_point.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct InfraredDataConfiguration {
  uint16_t carrierFrequency;
  uint16_t dutyCycle;
  uint16_t cycleCount;

  // Time in multiples of period defined by carrierFrequency
  struct PulseTime {
    FixedPoint<uint16_t, 8> onTime;
    FixedPoint<uint16_t, 8> offTime;
  };
  PulseTime header;
  PulseTime zeroBit;
  PulseTime oneBit;
  PulseTime trailer;

  const PulseTime &GetBitPulseTime(size_t index) const {
    return (&zeroBit)[index];
  }
};

enum InfraredProtocol {
  NEC,
  RC5,
  SAMSUNG,
  SONY12,
  SONY15,
  SONY20,
};

class Infrared {
public:
  static void SendData(uint64_t data, int bitCount,
                       const InfraredDataConfiguration &configuration);

  static void SendMessage(InfraredProtocol protocol, uint32_t d0, uint32_t d1,
                          uint32_t d2);

  static void Stop();

private:
  static void SendNECMessage(uint32_t address, uint32_t command);
  static void SendRC5Message(uint32_t address, uint32_t command,
                             uint32_t toggle);
  static void SendSamsungMessage(uint32_t address, uint32_t command);
  static void SendSony12Message(uint32_t address, uint32_t command);
  static void SendSony15Message(uint32_t address, uint32_t command);
  static void SendSony20Message(uint32_t address, uint32_t command,
                                uint32_t extension);
};

//---------------------------------------------------------------------------

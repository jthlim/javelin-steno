//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct InfraredDataConfiguration {
  uint16_t carrierFrequency;
  uint16_t dutyCycle;
  uint16_t loopCount;

  // Time in multiples of period defined by carrierFrequency
  struct PulseTime {
    uint16_t onTime;
    uint16_t offTime;
  };
  PulseTime header;
  PulseTime zeroBit;
  PulseTime oneBit;
  PulseTime trailer;
};

enum InfraredProtocol {
  RC5,
  NEC,
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
  static void SendRC5Message(uint32_t message);
};

//---------------------------------------------------------------------------

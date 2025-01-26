//---------------------------------------------------------------------------

#include "procat.h"
#include "../console.h"
#include "../hal/serial_port.h"

//---------------------------------------------------------------------------

struct StenoProcatPacket {
public:
  StenoProcatPacket(const StenoKeyState &state);

  uint8_t data[4];
};

StenoProcatPacket::StenoProcatPacket(const StenoKeyState &state)
    : data{0, 0, 0, 0xff} {
  uint32_t localKeyState = state.ToStroke().GetKeyState();

  while (localKeyState) {
    const uint32_t index = __builtin_ctzl(localKeyState) + 1;

    if (index < 24) {
      const uint32_t byte = index / 8;
      const uint32_t bit = (~index) & 7;
      data[byte] |= 1 << bit;
    }

    localKeyState &= localKeyState - 1;
  }
}

//---------------------------------------------------------------------------

void StenoProcat::Process(const StenoKeyState &value, StenoAction action) {
  next->Process(value, action);
  if (action != StenoAction::TRIGGER) {
    return;
  }

  ++counter;
  const StenoProcatPacket packet(value);
  SerialPort::SendData(packet.data, sizeof(packet.data));
}

void StenoProcat::PrintInfo() const {
  Console::Printf("  ProCAT: %zu strokes\n", counter);
}

//---------------------------------------------------------------------------

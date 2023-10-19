//---------------------------------------------------------------------------

#include "procat.h"
#include "../console.h"
#include "../serial_port.h"

//---------------------------------------------------------------------------

void StenoProcat::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }

  ++counter;

  uint32_t localKeyState = value.ToStroke().GetKeyState();
  uint8_t data[4] = {
      0,
      0,
      0,
      0xff,
  };

  while (localKeyState) {
    uint32_t index = __builtin_ctzl(localKeyState) + 1;

    uint32_t byte = index / 8;
    uint32_t bit = (~index) & 7;
    data[byte] |= 1 << bit;

    localKeyState &= localKeyState - 1;
  }

  SerialPort::SendData(data, sizeof(data));
}

void StenoProcat::PrintInfo() const {
  Console::Printf("  ProCAT: %zu strokes\n", counter);
}

//---------------------------------------------------------------------------

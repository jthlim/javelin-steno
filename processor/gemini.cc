//---------------------------------------------------------------------------

#include "gemini.h"
#include "../console.h"
#include "../serial_port.h"

//---------------------------------------------------------------------------

void StenoGemini::Process(const StenoKeyState &value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }

  ++counter;
  StenoGeminiPacket packet = value.ToGeminiPacket();
  SerialPort::SendData(packet.data, sizeof(packet.data));
}

void StenoGemini::PrintInfo() const {
  Console::Printf("  Gemini: %zu strokes\n", counter);
}

//---------------------------------------------------------------------------

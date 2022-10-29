//---------------------------------------------------------------------------

#include "gemini.h"
#include "../serial_port.h"

//---------------------------------------------------------------------------

void StenoGemini::Process(StenoKeyState value, StenoAction action) {
  if (action != StenoAction::TRIGGER) {
    return;
  }

  StenoGeminiPacket packet = value.ToGeminiPacket();
  SerialPort::SendData(packet.data, sizeof(packet.data));
}

//---------------------------------------------------------------------------

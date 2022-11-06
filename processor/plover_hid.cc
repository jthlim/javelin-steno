//---------------------------------------------------------------------------

#include "plover_hid.h"
#include "../console.h"

//---------------------------------------------------------------------------

void StenoPloverHid::Process(StenoKeyState value, StenoAction action) {
  switch (action) {
  case StenoAction::CANCEL:
  case StenoAction::TRIGGER:
    break;

  case StenoAction::PRESS:
  case StenoAction::RELEASE:
    StenoPloverHidPacket packet = value.ToPloverHidPacket();
    ++counter;
    SendPacket(packet);
  }
}

void StenoPloverHid::PrintInfo() const {
  Console::Printf("  Plover HID: %zu packets\n", counter);
}

//---------------------------------------------------------------------------

__attribute__((weak)) void
StenoPloverHid::SendPacket(const StenoPloverHidPacket &packet) {}

//---------------------------------------------------------------------------

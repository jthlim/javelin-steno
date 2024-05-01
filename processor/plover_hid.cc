//---------------------------------------------------------------------------

#include "plover_hid.h"
#include "../console.h"

//---------------------------------------------------------------------------

void StenoPloverHid::Process(const StenoKeyState &value, StenoAction action) {
  switch (action) {
  case StenoAction::CANCEL_KEY:
  case StenoAction::CANCEL_ALL:
  case StenoAction::TRIGGER:
    break;

  case StenoAction::PRESS:
  case StenoAction::RELEASE:
    const StenoPloverHidPacket packet = value.ToPloverHidPacket();
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

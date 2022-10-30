//---------------------------------------------------------------------------

#include "plover_hid.h"

//---------------------------------------------------------------------------

void StenoPloverHid::Process(StenoKeyState value, StenoAction action) {
  switch (action) {
  case StenoAction::CANCEL:
  case StenoAction::TRIGGER:
    break;

  case StenoAction::PRESS:
  case StenoAction::RELEASE:
    StenoPloverHidPacket packet = value.ToPloverHidPacket();
    SendPacket(packet);
  }
}

//---------------------------------------------------------------------------

__attribute__((weak)) void
StenoPloverHid::SendPacket(const StenoPloverHidPacket &packet) {}

//---------------------------------------------------------------------------

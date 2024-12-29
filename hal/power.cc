//---------------------------------------------------------------------------

#include "power.h"

//---------------------------------------------------------------------------

[[gnu::weak]] bool Power::IsCharging() { return false; }
[[gnu::weak]] int Power::GetBatteryPercentage() { return 0; }
[[gnu::weak]] bool Power::IsBoardPowered() { return true; }
[[gnu::weak]] void Power::SetBoardPower(bool on) {}

//---------------------------------------------------------------------------

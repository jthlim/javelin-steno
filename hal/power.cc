//---------------------------------------------------------------------------

#include "power.h"

//---------------------------------------------------------------------------

__attribute__((weak)) bool Power::IsCharging() { return false; }
__attribute__((weak)) int Power::GetBatteryPercentage() { return 0; }
__attribute__((weak)) void Power::SetBoardPower(bool on) {}

//---------------------------------------------------------------------------

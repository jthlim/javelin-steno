//---------------------------------------------------------------------------

#include "ble.h"

//---------------------------------------------------------------------------

__attribute__((weak)) bool Ble::IsConnected() { return false; }
__attribute__((weak)) void Ble::StartPairing() {}
__attribute__((weak)) int Ble::GetProfile() { return 0; }
__attribute__((weak)) void Ble::SetProfile(int profileId) {}
__attribute__((weak)) bool Ble::IsSleeping() { return false; }

//---------------------------------------------------------------------------

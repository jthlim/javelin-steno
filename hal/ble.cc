//---------------------------------------------------------------------------

#include "ble.h"

//---------------------------------------------------------------------------

__attribute__((weak)) bool Ble::IsConnected() { return false; }
__attribute__((weak)) void Ble::StartPairing() {}
__attribute__((weak)) bool Ble::IsPairConnected() { return false; }
__attribute__((weak)) int Ble::GetProfile() { return 0; }
__attribute__((weak)) void Ble::SetProfile(int profileId) {}
__attribute__((weak)) bool Ble::IsProfileConnected(int profileId) {
  return false;
}
__attribute__((weak)) void Ble::Disconnect() {}
__attribute__((weak)) bool Ble::IsSleeping() { return false; }
__attribute__((weak)) bool Ble::IsProfilePaired(int profileId) { return false; }
__attribute__((weak)) void Ble::Unpair() {}
__attribute__((weak)) bool Ble::IsProfileSleeping(int profileId) {
  return false;
}

__attribute__((weak)) bool Ble::IsAdvertising() { return false; }
__attribute__((weak)) bool Ble::IsScanning() { return false; }

__attribute__((weak)) KeyboardLedStatus Ble::GetActiveKeyboardLedStatus() {
  return KeyboardLedStatus(0);
}

//---------------------------------------------------------------------------

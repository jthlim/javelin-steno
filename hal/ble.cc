//---------------------------------------------------------------------------

#include "ble.h"

//---------------------------------------------------------------------------

[[gnu::weak]] bool Ble::IsConnected() { return false; }
[[gnu::weak]] void Ble::StartPairing() {}
[[gnu::weak]] bool Ble::IsPairConnected() { return false; }
[[gnu::weak]] int Ble::GetProfile() { return 0; }
[[gnu::weak]] void Ble::SetProfile(int profileId) {}
[[gnu::weak]] bool Ble::IsProfileConnected(int profileId) { return false; }
[[gnu::weak]] void Ble::Disconnect() {}
[[gnu::weak]] bool Ble::IsSleeping() { return false; }
[[gnu::weak]] bool Ble::IsProfilePaired(int profileId) { return false; }
[[gnu::weak]] void Ble::Unpair() {}
[[gnu::weak]] bool Ble::IsProfileSleeping(int profileId) { return false; }

[[gnu::weak]] bool Ble::IsAdvertising() { return false; }
[[gnu::weak]] bool Ble::IsScanning() { return false; }

[[gnu::weak]] KeyboardLedStatus Ble::GetActiveKeyboardLedStatus() {
  return KeyboardLedStatus(0);
}

//---------------------------------------------------------------------------

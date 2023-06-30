//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

class Ble {
public:
  static bool IsConnected();
  static void StartPairing();
  static bool IsPairConnected();
  static int GetProfile();
  static void SetProfile(int profileId);
  static bool IsProfileConnected(int profileId);
  static void Disconnect();
  static bool IsSleeping();
  static bool IsProfilePaired(int profileId);
  static void Unpair();
  static bool IsProfileSleeping(int profileId);
  static bool IsAdvertising();
  static bool IsScanning();
};

//---------------------------------------------------------------------------

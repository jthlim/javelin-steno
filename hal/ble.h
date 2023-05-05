//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

class Ble {
public:
  static bool IsConnected();
  static void StartPairing();
  static int GetProfile();
  static void SetProfile(int setId);
  static bool IsSleeping();
};

//---------------------------------------------------------------------------

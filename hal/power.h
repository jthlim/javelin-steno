//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

class Power {
public:
  static bool IsPowered();
  static bool IsCharging();
  static int GetBatteryPercentage();
  static void SetBoardPower(bool on);
};

//---------------------------------------------------------------------------

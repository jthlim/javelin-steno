//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

class Power {
public:
  static bool IsCharging();
  static int GetBatteryPercentage();
  static bool IsBoardPowered();
  static void SetBoardPower(bool on);
};

//---------------------------------------------------------------------------

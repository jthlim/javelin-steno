//---------------------------------------------------------------------------

#pragma once
#include "../date_time.h"

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

class RTC {
public:
  static bool HasValidDateTime();
  static DateTime GetDateTime();
  static void SetDateTime(const DateTime &dateTime);

  static bool DefaultHasValidDateTime();
  static DateTime DefaultGetDateTime();
  static void DefaultSetDateTime(const DateTime &dateTime);

  static void GetDateTimeValid_Binding();
  static void GetDateTime_Binding();

  static void AddConsoleCommands(Console &console);
  static uint32_t GetMillisecondsSinceLastSet();

private:
  bool isValid;
  uint32_t setTime;
  DateTime dateTime = DateTime{.day = 1, .month = 1, .year = 1970};

  static RTC instance;

  static void SetDateTime_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------

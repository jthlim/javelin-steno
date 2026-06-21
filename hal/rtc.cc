//---------------------------------------------------------------------------

#include "rtc.h"
#include "../clock.h"
#include "../console.h"
#include "../str.h"
#include "../unicode.h"

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

RTC RTC::instance;

//---------------------------------------------------------------------------

[[gnu::weak]] bool RTC::HasValidDateTime() { return DefaultHasValidDateTime(); }
[[gnu::weak]] DateTime RTC::GetDateTime() { return DefaultGetDateTime(); }
[[gnu::weak]] void RTC::SetDateTime(const DateTime &dateTime) {
  DefaultSetDateTime(dateTime);
}

bool RTC::DefaultHasValidDateTime() { return instance.isValid; }

uint32_t RTC::GetMillisecondsSinceLastSet() {
  return Clock::GetMilliseconds() - instance.setTime;
}

DateTime RTC::DefaultGetDateTime() {
  return instance.dateTime.AddSeconds(GetMillisecondsSinceLastSet() / 1000);
}

void RTC::DefaultSetDateTime(const DateTime &dateTime) {
  instance.dateTime = dateTime;
  instance.isValid = true;
  instance.setTime = Clock::GetMilliseconds();
}

void RTC::GetDateTimeValid_Binding() {
  Console::Printf("%d\n\n", HasValidDateTime());
}

void RTC::GetDateTime_Binding() {
  const DateTime dateTime = GetDateTime();
  dateTime.Printf(*ConsoleWriter::GetActiveWriter(), "%Y-%m-%d %H:%M:%S\n\n");
}

void RTC::SetDateTime_Binding(void *context, const char *commandLine) {
  const char *p = strchr(commandLine, ' ');
  int dayOfWeek = 0;

  if (!p) {
    Console::Printf("ERR Missing parameters\n\n");
    return;
  }
  for (int i = 0; i < 7; ++i) {
    if (Str::HasPrefix(p + 1, DateTime::SHORT_DAY_NAMES[i])) {
      dayOfWeek = i;
      p = strchr(p + 1, ' ');
      if (!p) {
        Console::Printf("ERR Missing parameters\n\n");
        return;
      }
      break;
    }
  }

  int parsedValues[6];
  for (int i = 0; i < 6; ++i) {
    while (!Unicode::IsAsciiDigit(*p)) {
      if (*p == '\0') {
        if (i == 1) {
          // Try to use it as a unix timestamp.
          SetDateTime(DateTime::Create(parsedValues[0]));
          Console::SendOk();
        } else {
          Console::Printf("ERR Missing parameters\n\n");
        }
        return;
      }
      ++p;
    }
    p = Str::ParseInteger(&parsedValues[i], p, false);
    if (!p) {
      Console::Printf("ERR Missing parameters\n\n");
      return;
    }
  }

  SetDateTime(DateTime{
      .seconds = uint8_t(parsedValues[5]),
      .minutes = uint8_t(parsedValues[4]),
      .hours = uint8_t(parsedValues[3]),
      .dayOfWeek = uint8_t(dayOfWeek),
      .day = uint8_t(parsedValues[2]),
      .month = uint8_t(parsedValues[1]),
      .year = uint16_t(parsedValues[0]),
  });

  Console::SendOk();
}

void RTC::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_date_time",
                          "Sets the current date & time, e.g. "
                          "\"set_date_time Thu 2026-05-15 17:30:15\"",
                          &SetDateTime_Binding, nullptr);
}

//---------------------------------------------------------------------------

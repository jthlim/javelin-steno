//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//---------------------------------------------------------------------------

class IWriter;

//---------------------------------------------------------------------------

struct DateTime {
  uint8_t seconds;   // 0-59
  uint8_t minutes;   // 0-59
  uint8_t hours;     // 0-23
  uint8_t dayOfWeek; // 0-6, 0 = Sunday
  uint8_t day;       // 1-31
  uint8_t month;     // 1-12
  uint16_t year;     // 1900-2099
  int32_t timezoneSeconds;

  static const char *const SHORT_DAY_NAMES[];
  static const char *const LONG_DAY_NAMES[];
  static const char *const SHORT_MONTH_NAMES[];
  static const char *const LONG_MONTH_NAMES[];

  static DateTime Create(uint32_t secondsSinceUnixEpoch,
                         int32_t timezoneSeconds);
  uint32_t GetSecondsSinceUnixEpoch() const;

  DateTime AddSeconds(uint32_t seconds) const;

  void Printf(IWriter &output, const char *format) const;

  bool operator==(const DateTime &other) const;
};
static_assert(sizeof(DateTime) == 12);

inline bool DateTime::operator==(const DateTime &other) const {
  // return seconds == other.seconds //
  //        && minutes == other.minutes //
  //        && hours == other.hours //
  //        && dayOfWeek == other.dayOfWeek //
  //        && day == other.day //
  //        && month == other.month //
  //        && year == other.year && timezoneSeconds == other.timezoneSeconds;

  return memcmp(this, &other, sizeof(*this)) == 0;
}

//---------------------------------------------------------------------------

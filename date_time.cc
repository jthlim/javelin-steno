//---------------------------------------------------------------------------

#include "date_time.h"
#include "clamp.h"
#include "writer.h"

//---------------------------------------------------------------------------

constexpr const char *DateTime::SHORT_DAY_NAMES[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

constexpr const char *DateTime::LONG_DAY_NAMES[] = {
    "Sunday",   "Monday", "Tuesday",  "Wednesday",
    "Thursday", "Friday", "Saturday",
};

constexpr const char *DateTime::SHORT_MONTH_NAMES[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

constexpr const char *DateTime::LONG_MONTH_NAMES[] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December",
};

constexpr uint8_t MONTH_DAYS_NO_LEAP_YEAR[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

constexpr uint8_t MONTH_DAYS_LEAP_YEAR[] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

//---------------------------------------------------------------------------

static bool IsLeapYear(uint32_t year) {
  if (year % 4 != 0) {
    return false;
  }
  return (year % 100 != 0) || (year % 400 == 0);
}

DateTime DateTime::Create(uint32_t secondsSinceUnixEpoch,
                          int32_t timezoneSeconds) {
  DateTime result;
  result.timezoneSeconds = timezoneSeconds;

  uint32_t daysRemaining = secondsSinceUnixEpoch / 86400;
  const uint32_t secondsInDay = secondsSinceUnixEpoch % 86400;
  result.seconds = secondsInDay % 60;
  result.minutes = (secondsInDay / 60) % 60;
  result.hours = secondsInDay / 3600;

  // Jan 1, 1970 was a Thursday (4)
  result.dayOfWeek = (4 + (daysRemaining % 7)) % 7;

  uint32_t currentYear = 1970;

  // Iterate through years
  while (true) {
    const uint32_t daysInYear = IsLeapYear(currentYear) ? 366 : 365;
    if (daysRemaining < daysInYear) {
      break;
    }
    daysRemaining -= daysInYear;
    currentYear++;
  }
  result.year = currentYear;

  const uint8_t *monthDays =
      IsLeapYear(currentYear) ? MONTH_DAYS_LEAP_YEAR : MONTH_DAYS_NO_LEAP_YEAR;

  for (int m = 0; m < 12; ++m) {
    if (daysRemaining < monthDays[m]) {
      result.month = m + 1;
      result.day = (uint8_t)(daysRemaining + 1);
      break;
    }
    daysRemaining -= monthDays[m];
  }

  return result;
}

uint32_t DateTime::GetSecondsSinceUnixEpoch() const {
  if (year < 1970) {
    return 0;
  }
  if (year > 2106) {
    return 0xffffffff;
  }

  uint64_t totalDays = day - 1;
  for (uint32_t y = 1970; y < year; ++y) {
    totalDays += IsLeapYear(y) ? 366 : 365;
  }

  const uint8_t *monthDays =
      IsLeapYear(year) ? MONTH_DAYS_LEAP_YEAR : MONTH_DAYS_NO_LEAP_YEAR;
  const int endMonth = ClampMax(month, 12);
  for (int m = 1; m < endMonth; ++m) {
    totalDays += monthDays[m - 1];
  }

  const int64_t totalSeconds =
      ((totalDays * 24 + hours) * 60 + minutes) * 60ll + seconds -
      timezoneSeconds;
  return (uint32_t)Clamp(totalSeconds, 0, 0xffffffff);
}

DateTime DateTime::AddSeconds(uint32_t seconds) const {
  DateTime result;
  result.timezoneSeconds = timezoneSeconds;

  const uint32_t totalSeconds = seconds + this->seconds;
  result.seconds = uint8_t(totalSeconds % 60);

  const uint32_t minutesCarry = totalSeconds / 60;
  const uint32_t minutes = minutesCarry + this->minutes;
  result.minutes = uint8_t(minutes % 60);

  const uint32_t hoursCarry = minutes / 60;
  const uint32_t hours = hoursCarry + this->hours;
  result.hours = uint8_t(hours % 24);

  uint32_t daysCarry = hours / 24;
  result.dayOfWeek = (dayOfWeek + daysCarry) % 7;

  result.day = day;
  result.month = month;
  result.year = year;

  while (daysCarry) {
    const uint8_t *monthDays = IsLeapYear(result.year)
                                   ? MONTH_DAYS_LEAP_YEAR
                                   : MONTH_DAYS_NO_LEAP_YEAR;
    const uint32_t remainingDaysInMonth =
        monthDays[result.month - 1] - result.day + 1;
    if (daysCarry < remainingDaysInMonth) {
      result.day += daysCarry;
      break;
    }

    result.day = 1;
    result.month++;
    if (result.month > 12) {
      result.month = 1;
      result.year++;
    }
    daysCarry -= remainingDaysInMonth;
  }

  return result;
}

// Formats the DateTime according to the specifiers
//
// %a	Short Weekday, e.g. "Sun"
// %A	Long Weekday, e.g. "Sunday"
// %w	Day of Week, 0..6 (Sunday is 0)
// %y	Short Year, e.g. "13"
// %Y	Long Year, e.g. "2013"
// %b	Short Month, e.g. "Jan"
// %B	Long Month, e.g. "January"
// %m	Month, 01..12
// %n	Month, 1..12
// %d	Day, 01..31
// %e	Day, 1..31
// %H	Hour, 00..23
// %i	Hour, 1..12
// %I	Hour, 01..12
// %M	Minute, 00..59
// %S	Seconds, 00..59
// %p	"AM" or "PM"
// %t	"a" or "p" for AM or PM.
// %T	"A" or "P" for AM or PM.
// %u	Unix Timestamp
// %z	Timezone in +hhmm format, e.g. -0600
// %%	%	Literal % character
void DateTime::Printf(IWriter &output, const char *format) const {
  for (;;)
    switch (const int c = *format++; c) {
    case '\0':
      return;

    case '%':
      switch (const int c = *format++; c) {
      case '\0':
        return;
      case 'a':
        output.WriteString(SHORT_DAY_NAMES[dayOfWeek]);
        break;
      case 'A':
        output.WriteString(LONG_DAY_NAMES[dayOfWeek]);
        break;
      case 'w':
        output.Printf("%d", dayOfWeek);
        break;
      case 'y':
        output.Printf("%02d", year % 100);
        break;
      case 'Y':
        output.Printf("%d", year);
        break;
      case 'b':
        output.WriteString(SHORT_MONTH_NAMES[month - 1]);
        break;
      case 'B':
        output.WriteString(LONG_MONTH_NAMES[month - 1]);
        break;
      case 'm':
        output.Printf("%02d", month);
        break;
      case 'n':
        output.Printf("%d", month);
        break;
      case 'd':
        output.Printf("%02d", day);
        break;
      case 'e':
        output.Printf("%d", day);
        break;
      case 'H':
        output.Printf("%02d", hours);
        break;
      case 'i':
        output.Printf("%d", (hours + 11) % 12 + 1);
        break;
      case 'I':
        output.Printf("%02d", (hours + 11) % 12 + 1);
        break;
      case 'M':
        output.Printf("%02d", minutes);
        break;
      case 'S':
        output.Printf("%02d", seconds);
        break;
      case 'p':
        output.WriteString(hours < 12 ? "AM" : "PM");
        break;
      case 't':
        output.WriteByte(hours < 12 ? 'a' : 'p');
        break;
      case 'T':
        output.WriteByte(hours < 12 ? 'A' : 'P');
        break;
      case 'u':
        output.Printf("%u", GetSecondsSinceUnixEpoch());
        break;
      case 'z': {
        int tzMinutes = timezoneSeconds / 60;
        int sign = '+';
        if (tzMinutes < 0) {
          tzMinutes = -tzMinutes;
          sign = '-';
        }
        output.Printf("%c%02u%02u", sign, tzMinutes / 60, tzMinutes % 60);
      } break;
      case '%':
        output.WriteByte('%');
        break;
      default:
        output.WriteByte('%');
        output.WriteByte(c);
        break;
      }
      break;

    case '\\':
      switch (const int c = *format++; c) {
      case '\0':
        return;
      case 'b':
        output.WriteByte('\b');
        break;
      case 'e':
        output.WriteByte('\e');
        break;
      case 'f':
        output.WriteByte('\f');
        break;
      case 'n':
        output.WriteByte('\n');
        break;
      case 'r':
        output.WriteByte('\r');
        break;
      case 't':
        output.WriteByte('\t');
        break;
      case '{':
      case '}':
      case '\\':
      case '\"':
      case '\'':
      case ' ':
      case ':':
        output.WriteByte(c);
        break;
      default:
        output.WriteByte('\\');
        output.WriteByte(c);
        break;
      }
      break;

    default:
      output.WriteByte(c);
      break;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include "str.h"
#include "unit_test.h"

//---------------------------------------------------------------------------

static void VerifyDate(uint32_t time, const DateTime &dateTime) {
  const DateTime a = DateTime::Create(time, 0);
  assert(a == dateTime);
}
static void VerifyDate(uint32_t time, uint16_t year, uint8_t month, uint8_t day,
                       uint8_t dayOfWeek, uint8_t hours, uint8_t minutes,
                       uint8_t seconds) {
  const DateTime a = DateTime::Create(time, 0);
  const DateTime b = DateTime{
      .seconds = seconds,
      .minutes = minutes,
      .hours = hours,
      .dayOfWeek = dayOfWeek,
      .day = day,
      .month = month,
      .year = year,
      .timezoneSeconds = 0,
  };
  assert(a == b);
}

TEST_BEGIN("DateTime: Verify DateTime::Create returns expected values") {
  VerifyDate(0, 1970, 1, 1, 4, 0, 0, 0);
  VerifyDate(1767225599, 2025, 12, 31, 3, 23, 59, 59);
}
TEST_END

TEST_BEGIN("DateTime: Verify DateTime::Add returns expected values") {
  const DateTime a = DateTime::Create(1767225599, 0);
  const DateTime b = a.AddSeconds(60 * 60 * 24 * 60 + 1);
  VerifyDate(1767225599 + 60 * 60 * 24 * 60 + 1, b);
}
TEST_END

TEST_BEGIN("DateTime: Printf produces expected results") {
  const DateTime a = DateTime::Create(1767225598, 0);
  BufferWriter writer;
  a.Printf(writer, "Time %Y-%m-%d %H:%M:%S");
  writer.WriteByte('\0');
  assert(Str::Eq(writer.GetBuffer(), "Time 2025-12-31 23:59:58"));
}
TEST_END

TEST_BEGIN(
    "DateTime: Verify GetSecondsSinceUnixEpoch returns expected values") {
  const DateTime epoch = DateTime::Create(0, 0);
  assert(epoch.GetSecondsSinceUnixEpoch() == 0);

  const DateTime dt1 = DateTime::Create(1767225599, 0);
  assert(dt1.GetSecondsSinceUnixEpoch() == 1767225599);

  const DateTime maxDt = DateTime::Create(0xffffffff, 0);
  assert(maxDt.GetSecondsSinceUnixEpoch() == 0xffffffff);

  const DateTime preEpoch = DateTime{.seconds = 59,
                                     .minutes = 59,
                                     .hours = 23,
                                     .dayOfWeek = 3,
                                     .day = 31,
                                     .month = 12,
                                     .year = 1969};
  assert(preEpoch.GetSecondsSinceUnixEpoch() == 0);

  const DateTime overflowDt = DateTime{.seconds = 0,
                                       .minutes = 0,
                                       .hours = 0,
                                       .dayOfWeek = 0,
                                       .day = 1,
                                       .month = 1,
                                       .year = 2200};
  assert(overflowDt.GetSecondsSinceUnixEpoch() == 0xffffffff);
}
TEST_END

//---------------------------------------------------------------------------

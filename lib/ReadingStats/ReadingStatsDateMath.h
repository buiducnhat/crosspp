#pragma once

#include <cstdint>
#include <cstdio>

namespace ReadingStats {

// Number of days retained in the daily aggregate.
inline constexpr int MAX_DAYS = 365;

// Civil-date math (Howard Hinnant's algorithms, public domain). Pure
// functions so they are unit-testable on host without RTC hardware.

inline int32_t daysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int>(doe) - 719468;
}

inline void civilFromDays(int32_t z, int& y, unsigned& m, unsigned& d) {
  z += 719468;
  const int era = (z >= 0 ? z : z - 146096) / 146097;
  const unsigned doe = static_cast<unsigned>(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  y = static_cast<int>(yoe) + era * 400;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  d = doy - (153 * mp + 2) / 5 + 1;
  m = mp + (mp < 10 ? 3 : -9);
  y += m <= 2;
}

// Formats dayNumber (days since 1970-01-01) as "YYYY-MM-DD" into buf
// (needs >= 11 bytes). Returns false on bad buffer.
inline bool formatDayKey(int32_t dayNumber, char* buf, unsigned bufSize) {
  if (bufSize < 11) return false;
  int y;
  unsigned mo, d;
  civilFromDays(dayNumber, y, mo, d);
  snprintf(buf, bufSize, "%04d-%02u-%02u", y, mo, d);
  return true;
}

// Parses "YYYY-MM-DD" into a day number. Returns false on malformed input.
inline bool parseDayKey(const char* key, int32_t& dayNumber) {
  if (!key) return false;
  int y, mo, d;
  if (sscanf(key, "%d-%d-%d", &y, &mo, &d) != 3) return false;
  if (mo < 1 || mo > 12 || d < 1 || d > 31) return false;
  dayNumber = daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d));
  return true;
}

// Weekday of a day number: 0=Sunday .. 6=Saturday (1970-01-01 was Thursday).
inline int weekdayOfDay(int32_t dayNumber) {
  int wd = (dayNumber + 4) % 7;
  if (wd < 0) wd += 7;
  return wd;
}

// Minutes-per-day -> heatmap intensity level 0-4 (4 = darkest, >=60 min).
inline int intensityLevel(uint32_t minutes) {
  if (minutes == 0) return 0;
  if (minutes < 15) return 1;
  if (minutes < 30) return 2;
  if (minutes < 60) return 3;
  return 4;
}

}  // namespace ReadingStats

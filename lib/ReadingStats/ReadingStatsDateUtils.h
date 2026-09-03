#pragma once

#include <Arduino.h>
#include <HalClock.h>

#include <cstdint>
#include <ctime>

#include "ReadingStatsDateMath.h"

namespace ReadingStats {

// Current UTC epoch seconds: RTC if present, else time() (valid after NTP
// sync), else 0.
inline uint32_t nowEpoch() {
  const uint32_t rtcEpoch = halClock.getEpoch();
  if (rtcEpoch != 0) return rtcEpoch;
  const time_t t = time(nullptr);
  // Consider time() valid only if it reports a date past 2024-01-01.
  if (t > 1704067200) return static_cast<uint32_t>(t);
  return 0;
}

// Current day number. When no real clock exists, synthesizes a day from
// uptime so sessions still aggregate (dates are placeholders until NTP/RTC).
inline int32_t todayDayNumber() {
  const uint32_t epoch = nowEpoch();
  if (epoch != 0) return static_cast<int32_t>(epoch / 86400);
  return static_cast<int32_t>(millis() / 86400000UL);
}

}  // namespace ReadingStats

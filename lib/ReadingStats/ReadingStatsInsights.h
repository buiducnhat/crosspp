#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "ReadingStatsTypes.h"

namespace ReadingStats {

// Habit metrics derived on the fly from the daily aggregate — no extra
// persisted state. Pure computation over the retained days (<= MAX_DAYS),
// so it runs once on screen entry, never per frame.
struct Insights {
  uint32_t currentStreak = 0;     // consecutive days with minutes > 0 ending today/yesterday
  uint32_t longestStreak = 0;     // longest run within the retained window
  uint32_t activeDays = 0;        // days with minutes > 0
  uint32_t avgMinutes7 = 0;       // mean minutes/day over the last 7 days
  uint32_t avgMinutes30 = 0;      // mean minutes/day over the last 30 days
  int32_t bestDayNumber = -1;     // day with most minutes; -1 when no data
  uint32_t bestDayMinutes = 0;
  int bestWeekday = -1;           // 0=Sunday..6=Saturday with most minutes; -1 when no data
  uint32_t avgSessionMinutes = 0; // totalMinutes / sessions over retained days
};

Insights computeInsights(const std::map<std::string, DayStats>& days, int32_t today);

}  // namespace ReadingStats

#include "ReadingStatsInsights.h"

#include "ReadingStatsDateMath.h"

namespace ReadingStats {

Insights computeInsights(const std::map<std::string, DayStats>& days, const int32_t today) {
  Insights out;
  if (days.empty()) return out;

  // Day lookup by day number: keys are "YYYY-MM-DD" strings, so a numeric
  // probe needs the formatted key. 366 probes is trivial next to parsing.
  auto minutesOf = [&days](const int32_t day) -> uint32_t {
    char key[11];
    if (!formatDayKey(day, key, sizeof(key))) return 0;
    const auto it = days.find(key);
    return it == days.end() ? 0 : it->second.minutes;
  };

  uint32_t totalMinutes = 0;
  uint32_t totalSessions = 0;
  uint32_t weekdayMinutes[7] = {};
  uint32_t sum7 = 0;
  uint32_t sum30 = 0;
  uint32_t run = 0;

  for (int32_t day = today - (MAX_DAYS - 1); day <= today; day++) {
    const uint32_t m = minutesOf(day);
    if (m == 0) {
      if (run > out.longestStreak) out.longestStreak = run;
      run = 0;
      continue;
    }
    run++;
    out.activeDays++;
    totalMinutes += m;
    if (today - day < 7) sum7 += m;
    if (today - day < 30) sum30 += m;
    weekdayMinutes[weekdayOfDay(day)] += m;
    if (m > out.bestDayMinutes) {
      out.bestDayMinutes = m;
      out.bestDayNumber = day;
    }
  }
  if (run > out.longestStreak) out.longestStreak = run;

  // Current streak: a today with no reading yet does not break the chain —
  // count back from the most recent active day instead.
  int32_t anchor = today;
  if (minutesOf(today) == 0) anchor = today - 1;
  while (minutesOf(anchor) > 0) {
    out.currentStreak++;
    anchor--;
  }

  for (const auto& kv : days) totalSessions += kv.second.sessions;

  out.avgMinutes7 = sum7 / 7;
  out.avgMinutes30 = sum30 / 30;
  out.avgSessionMinutes = totalSessions > 0 ? totalMinutes / totalSessions : 0;

  int bestWd = -1;
  uint32_t bestWdMinutes = 0;
  for (int wd = 0; wd < 7; wd++) {
    if (weekdayMinutes[wd] > bestWdMinutes) {
      bestWdMinutes = weekdayMinutes[wd];
      bestWd = wd;
    }
  }
  out.bestWeekday = bestWd;

  return out;
}

}  // namespace ReadingStats

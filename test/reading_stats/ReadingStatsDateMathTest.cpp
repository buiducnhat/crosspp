#include <gtest/gtest.h>

#include <cstring>

#include "ReadingStats/ReadingStatsDateMath.h"

using namespace ReadingStats;

TEST(ReadingStatsDateMath, EpochBoundaryRoundTrips) {
  EXPECT_EQ(daysFromCivil(1970, 1, 1), 0);
  int y;
  unsigned m, d;
  civilFromDays(0, y, m, d);
  EXPECT_EQ(y, 1970);
  EXPECT_EQ(m, 1u);
  EXPECT_EQ(d, 1u);
}

TEST(ReadingStatsDateMath, KnownDates) {
  EXPECT_EQ(daysFromCivil(2024, 1, 1), 19723);   // 1704067200 / 86400
  EXPECT_EQ(daysFromCivil(2000, 2, 29), 11016);  // leap day
  EXPECT_EQ(daysFromCivil(2026, 9, 1), 20697);
}

TEST(ReadingStatsDateMath, LeapYearBoundaries) {
  // Feb 29 -> Mar 1 transition in a leap year and a non-leap year.
  const int32_t feb29 = daysFromCivil(2024, 2, 29);
  EXPECT_EQ(daysFromCivil(2024, 3, 1), feb29 + 1);
  const int32_t feb28_2025 = daysFromCivil(2025, 2, 28);
  EXPECT_EQ(daysFromCivil(2025, 3, 1), feb28_2025 + 1);
}

TEST(ReadingStatsDateMath, FormatParseRoundTrip) {
  char buf[11];
  ASSERT_TRUE(formatDayKey(20697, buf, sizeof(buf)));
  EXPECT_STREQ(buf, "2026-09-01");
  int32_t day = -1;
  ASSERT_TRUE(parseDayKey(buf, day));
  EXPECT_EQ(day, 20697);
}

TEST(ReadingStatsDateMath, ParseRejectsMalformed) {
  int32_t day;
  EXPECT_FALSE(parseDayKey(nullptr, day));
  EXPECT_FALSE(parseDayKey("2026-13-01", day));
  EXPECT_FALSE(parseDayKey("2026-00-10", day));
  EXPECT_FALSE(parseDayKey("2026-01-32", day));
  EXPECT_FALSE(parseDayKey("not-a-date", day));
}

TEST(ReadingStatsDateMath, WeekdayMatchesKnownDates) {
  EXPECT_EQ(weekdayOfDay(daysFromCivil(1970, 1, 1)), 4);   // Thursday
  EXPECT_EQ(weekdayOfDay(daysFromCivil(2026, 9, 1)), 2);   // Tuesday
  EXPECT_EQ(weekdayOfDay(daysFromCivil(2026, 8, 30)), 0);  // Sunday
}

TEST(ReadingStatsDateMath, IntensityLevels) {
  EXPECT_EQ(intensityLevel(0), 0);
  EXPECT_EQ(intensityLevel(14), 1);
  EXPECT_EQ(intensityLevel(15), 2);
  EXPECT_EQ(intensityLevel(29), 2);
  EXPECT_EQ(intensityLevel(30), 3);
  EXPECT_EQ(intensityLevel(59), 3);
  EXPECT_EQ(intensityLevel(60), 4);
  EXPECT_EQ(intensityLevel(600), 4);
}

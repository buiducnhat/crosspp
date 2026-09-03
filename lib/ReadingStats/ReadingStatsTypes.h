#pragma once

#include <cstdint>

namespace ReadingStats {

// Per-day aggregate.
struct DayStats {
  uint32_t minutes = 0;
  uint32_t sessions = 0;
  uint32_t completed = 0;
};

// Per-book aggregate (keyed by book path in the store).
struct BookStats {
  uint32_t totalMinutes = 0;
  uint32_t activeDays = 0;
  uint32_t sessions = 0;
  uint32_t completed = 0;
};

}  // namespace ReadingStats

#include "ReadingSessionTracker.h"

#include <Arduino.h>
#include <Logging.h>

#include "ReadingStatsDateUtils.h"
#include "ReadingStatsStore.h"

namespace {
constexpr const char* TAG = "RSTATS";
}

void ReadingSessionTracker::flushToStore(bool save) {
  if (pendingMs == 0 && !completedCounted) return;
  char dayKey[11];
  if (!ReadingStats::formatDayKey(sessionDay, dayKey, sizeof(dayKey))) return;
  STATS_STORE.recordSession(currentBookPath.c_str(), dayKey, sessionDay, pendingMs / 60000, completedCounted);
  pendingMs = 0;
  completedCounted = false;
  if (save && !STATS_STORE.saveToFile()) {
    LOG_ERR(TAG, "Failed to save reading stats");
  }
}

void ReadingSessionTracker::beginSession(const std::string& bookPath) {
  if (inSession) endSession();

  currentBookPath = bookPath;
  sessionDay = ReadingStats::todayDayNumber();
  activeMs = 0;
  pendingMs = 0;
  completedCounted = false;
  lastActivityMs = millis();
  inSession = true;
}

void ReadingSessionTracker::noteActivity() {
  if (!inSession) return;
  const unsigned long now = millis();
  const unsigned long gap = now - lastActivityMs;  // unsigned wrap handles millis() rollover
  // Only the active window since the previous activity counts; longer gaps
  // are idle time and excluded entirely.
  activeMs += gap < ACTIVITY_TIMEOUT_MS ? gap : ACTIVITY_TIMEOUT_MS;
  lastActivityMs = now;
}

void ReadingSessionTracker::endSession() {
  if (!inSession) return;
  noteActivity();
  inSession = false;

  // Discard noise sessions shorter than one minute (open + immediate close).
  if (activeMs < 60000UL && !completedCounted) {
    LOG_DBG(TAG, "Discarding %lus session for %s", static_cast<unsigned long>(activeMs / 1000),
            currentBookPath.c_str());
    return;
  }

  pendingMs = activeMs;
  flushToStore(true);
  LOG_DBG(TAG, "Recorded %lum session for %s", static_cast<unsigned long>(activeMs / 60000), currentBookPath.c_str());
  currentBookPath.clear();
}

void ReadingSessionTracker::update() {
  if (!inSession) return;

  const unsigned long now = millis();
  if (now - lastActivityMs > ACTIVITY_TIMEOUT_MS) {
    // Window expired: bank the accumulated active time and re-anchor the
    // window at now so idle time past this point is not counted.
    pendingMs += activeMs;
    activeMs = 0;
    lastActivityMs = now;

    static unsigned long lastFlushMs = 0;
    if (pendingMs > 0 && now - lastFlushMs >= FLUSH_INTERVAL_MS) {
      flushToStore(true);
      lastFlushMs = now;
    }
  }
}

#pragma once

#include <cstdint>
#include <string>

// Tracks one active reading session and records it into ReadingStatsStore.
//
// Heuristic: a session is opened when a book opens (beginSession) and time is
// counted only while the reader shows activity. Each noteActivity() extends
// the active window by ACTIVITY_TIMEOUT_MS; gaps longer than that are idle
// time and not counted. The session is closed by endSession() (book closed /
// activity exit), which records the aggregate into the store and flushes SD.
//
// Usage (ReaderActivity base):
//   onEnter: tracker.beginSession(bookPath)
//   on activity (page turn etc.): tracker.noteActivity()
//   onExit / book switch: tracker.endSession()
//   when book finished: tracker.markCompleted() (latched until endSession)
//
// Not thread-safe: called from the main task only.
class ReadingSessionTracker {
  static constexpr unsigned long ACTIVITY_TIMEOUT_MS = 5UL * 60UL * 1000UL;  // 5 minutes
  // Periodic SD flush for long uninterrupted sessions.
  static constexpr unsigned long FLUSH_INTERVAL_MS = 5UL * 60UL * 1000UL;

  std::string currentBookPath;
  int32_t sessionDay = -1;  // day number when the session opened
  uint32_t activeMs = 0;    // accumulated active milliseconds
  uint32_t pendingMs = 0;   // active ms not yet recorded into the store
  unsigned long lastActivityMs = 0;
  bool inSession = false;
  bool completedCounted = false;

  // Moves pendingMs into the store. Saves to SD only when save=true.
  void flushToStore(bool save);

 public:
  // Starts a session for bookPath; closes any dangling session first.
  void beginSession(const std::string& bookPath);

  // Marks user activity (page turn, menu open). Extends the active window.
  void noteActivity();

  // Latches the completed flag; applied when the session closes.
  void markCompleted() { completedCounted = true; }

  // Closes the session, records stats, and flushes SD.
  void endSession();

  // Call periodically from loop(): expires the active window after
  // ACTIVITY_TIMEOUT_MS of no activity and performs periodic flushes.
  void update();

  bool isInSession() const { return inSession; }
};

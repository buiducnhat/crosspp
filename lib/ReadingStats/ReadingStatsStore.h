#pragma once

#include <ArduinoJson.h>
#include <PersistableStore.h>

#include <cstdint>
#include <map>
#include <string>

#include "ReadingStatsTypes.h"

// Daily reading statistics persisted to /.crosspoint/reading_stats.json.
//
// JSON v1 layout:
//   { "v": 1,
//     "days":  { "YYYY-MM-DD": { "m": minutes, "s": sessions, "c": completed } },
//     "books": { "<path>": { "t": totalMinutes, "a": activeDays, "s": sessions, "c": completed } } }
//
// Reads/writes go through PersistableStoreBase helpers (flash-neutral JSON).
class ReadingStatsStore : public PersistableStore<ReadingStatsStore> {
 private:
  // dayKey ("YYYY-MM-DD") -> stats. std::map keeps keys sorted so pruning the
  // oldest entries is trivial and iteration order is chronological.
  std::map<std::string, ReadingStats::DayStats> days;
  std::map<std::string, ReadingStats::BookStats> books;
  // Last day (day number) each book saw activity; used to bump activeDays
  // once per book per day.
  std::map<std::string, int32_t> bookLastActiveDay;

  // Set when loadFromFile() has run (or been attempted) so mutators can
  // lazily load before touching in-memory state.
  bool loaded = false;

  ReadingStatsStore() = default;
  ~ReadingStatsStore() = default;

  friend class PersistableStore<ReadingStatsStore>;

  void ensureLoaded();
  void pruneOldDays();

 public:
  static const char* getFilePath() { return "/.crosspoint/reading_stats.json"; }
  void toJson(JsonDocument& doc) const;
  bool fromJson(JsonVariantConst doc);

  // Records a finished session. minutes = active minutes, completed = book
  // finished during this session. dayKey must be "YYYY-MM-DD" for
  // dayNumber (see ReadingStatsDateUtils.h). Does not persist — caller
  // decides when to saveToFile() (debounced).
  void recordSession(const char* bookPath, const char* dayKey, int32_t dayNumber, uint32_t minutes, bool completed);

  // Snapshot accessors for the UI. Totals are computed over retained days.
  const std::map<std::string, ReadingStats::DayStats>& getDays();
  const std::map<std::string, ReadingStats::BookStats>& getBooks();

  // Day stats for a specific day number, or nullptr when no data.
  const ReadingStats::DayStats* getDay(int32_t dayNumber);

  // Book stats for a specific path, or nullptr when no data.
  const ReadingStats::BookStats* getBook(const char* path);

  struct Totals {
    uint32_t minutes = 0;
    uint32_t sessions = 0;
    uint32_t completed = 0;
  };
  Totals getTotals();
};

#define STATS_STORE ReadingStatsStore::getInstance()

#include "ReadingStatsStore.h"

#include <Logging.h>

#include "ReadingStatsDateMath.h"

namespace {
constexpr const char* TAG = "RSTATS";
}

void ReadingStatsStore::ensureLoaded() {
  if (loaded) return;
  loaded = true;
  // Missing file on first boot is expected; readDocFromFile stays silent.
  loadFromFile();
}

void ReadingStatsStore::pruneOldDays() {
  while (static_cast<int>(days.size()) > ReadingStats::MAX_DAYS) {
    days.erase(days.begin());
  }
}

void ReadingStatsStore::recordSession(const char* bookPath, const char* dayKey, int32_t dayNumber, uint32_t minutes,
                                      bool completed) {
  ensureLoaded();

  auto& day = days[dayKey];
  day.minutes += minutes;
  day.sessions += 1;
  if (completed) day.completed += 1;
  pruneOldDays();

  if (bookPath && bookPath[0] != '\0') {
    auto& book = books[bookPath];
    book.totalMinutes += minutes;
    book.sessions += 1;
    if (completed) book.completed += 1;
    auto it = bookLastActiveDay.find(bookPath);
    if (it == bookLastActiveDay.end() || it->second != dayNumber) {
      book.activeDays += 1;
      bookLastActiveDay[bookPath] = dayNumber;
    }
  }
}

const std::map<std::string, ReadingStats::DayStats>& ReadingStatsStore::getDays() {
  ensureLoaded();
  return days;
}

const std::map<std::string, ReadingStats::BookStats>& ReadingStatsStore::getBooks() {
  ensureLoaded();
  return books;
}

const ReadingStats::DayStats* ReadingStatsStore::getDay(int32_t dayNumber) {
  ensureLoaded();
  char key[11];
  if (!ReadingStats::formatDayKey(dayNumber, key, sizeof(key))) return nullptr;
  const auto it = days.find(key);
  return it == days.end() ? nullptr : &it->second;
}

const ReadingStats::BookStats* ReadingStatsStore::getBook(const char* path) {
  ensureLoaded();
  const auto it = books.find(path);
  return it == books.end() ? nullptr : &it->second;
}

ReadingStatsStore::Totals ReadingStatsStore::getTotals() {
  ensureLoaded();
  Totals t;
  for (const auto& kv : days) {
    t.minutes += kv.second.minutes;
    t.sessions += kv.second.sessions;
    t.completed += kv.second.completed;
  }
  return t;
}

void ReadingStatsStore::toJson(JsonDocument& doc) const {
  doc["v"] = 1;
  JsonObject daysObj = doc["days"].to<JsonObject>();
  for (const auto& kv : days) {
    JsonObject d = daysObj[kv.first].to<JsonObject>();
    d["m"] = kv.second.minutes;
    d["s"] = kv.second.sessions;
    d["c"] = kv.second.completed;
  }
  JsonObject booksObj = doc["books"].to<JsonObject>();
  for (const auto& kv : books) {
    JsonObject b = booksObj[kv.first].to<JsonObject>();
    b["t"] = kv.second.totalMinutes;
    b["a"] = kv.second.activeDays;
    b["s"] = kv.second.sessions;
    b["c"] = kv.second.completed;
  }
}

bool ReadingStatsStore::fromJson(JsonVariantConst doc) {
  if (!doc.is<JsonObjectConst>()) return false;
  const int version = doc["v"] | 0;
  if (version != 1) {
    LOG_ERR(TAG, "Unsupported reading_stats.json version %d", version);
    return false;
  }

  days.clear();
  books.clear();
  bookLastActiveDay.clear();

  JsonObjectConst daysObj = doc["days"];
  for (JsonPairConst kv : daysObj) {
    int32_t dayNumber;
    if (!ReadingStats::parseDayKey(kv.key().c_str(), dayNumber)) continue;
    ReadingStats::DayStats d;
    d.minutes = kv.value()["m"] | 0u;
    d.sessions = kv.value()["s"] | 0u;
    d.completed = kv.value()["c"] | 0u;
    days.emplace(kv.key().c_str(), d);
  }
  pruneOldDays();

  JsonObjectConst booksObj = doc["books"];
  for (JsonPairConst kv : booksObj) {
    ReadingStats::BookStats b;
    b.totalMinutes = kv.value()["t"] | 0u;
    b.activeDays = kv.value()["a"] | 0u;
    b.sessions = kv.value()["s"] | 0u;
    b.completed = kv.value()["c"] | 0u;
    books.emplace(kv.key().c_str(), b);
  }
  return true;
}

#include "ReadingStatsActivity.h"

#include <GfxRenderer.h>
#include <HalDisplay.h>
#include <I18n.h>
#include <Logging.h>
#include <MappedInputManager.h>
#include <ReadingStatsDateUtils.h>
#include <ReadingStatsInsights.h>
#include <RecentBooksStore.h>

#include <cstdio>

#include "components/UITheme.h"
#include "components/UiAppHelpers.h"

namespace fui = freeink::ui;

ReadingStatsActivity::ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : UiTabListActivity("ReadingStats", renderer, mappedInput) {}

void ReadingStatsActivity::onEnter() {
  UiTabListActivity::onEnter();

  // Heatmap opens on the newest page (today is in the last column there).
  const int32_t today = ReadingStats::todayDayNumber();
  heatPage = (HEATMAP_WEEKS - 1) / HEATMAP_PAGE_WEEKS;
  heatCursorWeek = (HEATMAP_WEEKS - 1) % HEATMAP_PAGE_WEEKS;
  heatCursorRow = ReadingStats::weekdayOfDay(today);

  rebuildBookItems();
}

int ReadingStatsActivity::listCount() const {
  // Overview and Heatmap are single-element tabs; By Book lists the books.
  if (selectedTab == 1) return static_cast<int>(statBookPaths.size());
  return 1;
}

const char* ReadingStatsActivity::tabLabel(const int index) const {
  static const StrId tabNames[] = {StrId::STR_STATS_TAB_OVERVIEW, StrId::STR_STATS_TAB_BY_BOOK,
                                   StrId::STR_STATS_TAB_HEATMAP, StrId::STR_STATS_TAB_INSIGHTS};
  return I18N.get(tabNames[index]);
}

const char* ReadingStatsActivity::headerTitle() const { return tr(STR_READING_STATS_TITLE); }

const char* ReadingStatsActivity::minutesText(const uint32_t minutes, char* buf, const size_t bufSize) {
  if (minutes >= 60) {
    snprintf(buf, bufSize, "%luh %02lum", static_cast<unsigned long>(minutes / 60),
             static_cast<unsigned long>(minutes % 60));
  } else {
    snprintf(buf, bufSize, "%lum", static_cast<unsigned long>(minutes));
  }
  return buf;
}

void ReadingStatsActivity::rebuildBookItems() {
  statBookPaths.clear();
  statBookPaths.reserve(RECENT_BOOKS.getCount());
  for (const auto& book : RECENT_BOOKS.getBooks()) {
    if (STATS_STORE.getBook(book.path.c_str()) != nullptr) {
      statBookPaths.push_back(book.path);
    }
  }

  bookTitles.assign(statBookPaths.size(), std::string());
  bookValues.assign(statBookPaths.size(), std::string());
  bookItems.clear();
  bookItems.reserve(statBookPaths.size());
  for (size_t i = 0; i < statBookPaths.size(); i++) {
    fui::ListItem item;
    item.actionValue = static_cast<int16_t>(i);
    bookItems.push_back(item);
  }
}

void ReadingStatsActivity::buildScreen(UiScreen& screen) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  screen.setContentMarginFromScreen(fui::Insets{static_cast<int16_t>(metrics.topPadding + metrics.headerHeight), 0,
                                                static_cast<int16_t>(metrics.buttonHintsHeight), 0});

  buildTabBar(screen);

  switch (selectedTab) {
    case 0:
      buildOverview(screen);
      break;
    case 1:
      buildByBook(screen);
      break;
    case 2:
      buildHeatmap(screen);
      break;
    default:
      buildInsights(screen);
      break;
  }
}

void ReadingStatsActivity::buildOverview(UiScreen& screen) {
  const auto totals = STATS_STORE.getTotals();
  const auto insights = ReadingStats::computeInsights(STATS_STORE.getDays(), ReadingStats::todayDayNumber());
  char timeBuf[24];
  char sessionsBuf[12];
  char completedBuf[12];
  char streakBuf[24];
  char avgBuf[24];
  snprintf(sessionsBuf, sizeof(sessionsBuf), "%lu", static_cast<unsigned long>(totals.sessions));
  snprintf(completedBuf, sizeof(completedBuf), "%lu", static_cast<unsigned long>(totals.completed));
  snprintf(streakBuf, sizeof(streakBuf), "%lu %s", static_cast<unsigned long>(insights.currentStreak),
           insights.currentStreak == 1 ? tr(STR_STATS_DAY_UNIT) : tr(STR_STATS_DAYS_UNIT));

  // Labels include the value: TileGridItem carries a single label string.
  char tile0[48];
  char tile1[48];
  char tile2[48];
  char tile3[48];
  char tile4[48];
  snprintf(tile0, sizeof(tile0), "%s: %s", tr(STR_STATS_TOTAL_TIME),
           minutesText(totals.minutes, timeBuf, sizeof(timeBuf)));
  snprintf(tile1, sizeof(tile1), "%s: %s", tr(STR_STATS_SESSIONS), sessionsBuf);
  snprintf(tile2, sizeof(tile2), "%s: %s", tr(STR_STATS_BOOKS_COMPLETED), completedBuf);
  snprintf(tile3, sizeof(tile3), "%s: %s", tr(STR_STATS_CURRENT_STREAK), streakBuf);
  snprintf(tile4, sizeof(tile4), "%s: %s", tr(STR_STATS_AVG_7_DAYS),
           minutesText(insights.avgMinutes7, avgBuf, sizeof(avgBuf)));

  fui::TileGridItem items[5];
  items[0].label = tile0;
  items[1].label = tile1;
  items[2].label = tile2;
  items[3].label = tile3;
  items[4].label = tile4;

  fui::TileGridProps props;
  props.items = items;
  props.count = 5;
  props.action = fui::NO_ACTION;
  props.inputMask = 0;  // passive display; buttons navigate the ring only
  screen.tileGrid(props);
}

void ReadingStatsActivity::buildInsights(UiScreen& screen) {
  const auto insights = ReadingStats::computeInsights(STATS_STORE.getDays(), ReadingStats::todayDayNumber());
  if (insights.activeDays == 0) {
    screen.centeredText(tr(STR_STATS_NO_DATA));
    return;
  }

  static const StrId weekdayNames[] = {StrId::STR_STATS_DAY_SUN, StrId::STR_STATS_DAY_MON, StrId::STR_STATS_DAY_TUE,
                                       StrId::STR_STATS_DAY_WED, StrId::STR_STATS_DAY_THU, StrId::STR_STATS_DAY_FRI,
                                       StrId::STR_STATS_DAY_SAT};

  // Render passes re-run buildScreen, so values are formatted into member
  // buffers that stay valid until the next build.
  snprintf(insightValues[0], INSIGHT_VALUE_SIZE, "%lu %s", static_cast<unsigned long>(insights.currentStreak),
           insights.currentStreak == 1 ? tr(STR_STATS_DAY_UNIT) : tr(STR_STATS_DAYS_UNIT));
  snprintf(insightValues[1], INSIGHT_VALUE_SIZE, "%lu %s", static_cast<unsigned long>(insights.longestStreak),
           insights.longestStreak == 1 ? tr(STR_STATS_DAY_UNIT) : tr(STR_STATS_DAYS_UNIT));
  minutesText(insights.avgMinutes7, insightValues[2], INSIGHT_VALUE_SIZE);
  minutesText(insights.avgMinutes30, insightValues[3], INSIGHT_VALUE_SIZE);
  if (insights.bestDayNumber >= 0) {
    char dateBuf[11];
    ReadingStats::formatDayKey(insights.bestDayNumber, dateBuf, sizeof(dateBuf));
    char minBuf[24];
    snprintf(insightValues[4], INSIGHT_VALUE_SIZE, "%s (%s)",
             minutesText(insights.bestDayMinutes, minBuf, sizeof(minBuf)), dateBuf);
  }
  minutesText(insights.avgSessionMinutes, insightValues[6], INSIGHT_VALUE_SIZE);

  insightLabels[0] = tr(STR_STATS_CURRENT_STREAK);
  insightLabels[1] = tr(STR_STATS_LONGEST_STREAK);
  insightLabels[2] = tr(STR_STATS_AVG_7_DAYS);
  insightLabels[3] = tr(STR_STATS_AVG_30_DAYS);
  insightLabels[4] = tr(STR_STATS_BEST_DAY);
  insightLabels[5] = tr(STR_STATS_FAVORITE_WEEKDAY);
  insightLabels[6] = tr(STR_STATS_AVG_SESSION);
  if (insights.bestWeekday >= 0) {
    snprintf(insightValues[5], INSIGHT_VALUE_SIZE, "%s", I18N.get(weekdayNames[insights.bestWeekday]));
  }

  for (int i = 0; i < INSIGHT_ROWS; i++) {
    insightItems[i].label = insightLabels[i];
    insightItems[i].value = insightValues[i];
  }

  fui::ListProps props;
  props.items = insightItems;
  props.count = INSIGHT_ROWS;
  props.action = fui::NO_ACTION;
  props.inputMask = 0;  // passive list; buttons navigate the ring only
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  syncTabListViewport(screen, props);
  screen.list(props);
}

void ReadingStatsActivity::buildByBook(UiScreen& screen) {
  if (statBookPaths.empty()) {
    screen.centeredText(tr(STR_STATS_NO_DATA));
    return;
  }

  for (size_t i = 0; i < statBookPaths.size(); i++) {
    const auto& path = statBookPaths[i];
    const auto* stats = STATS_STORE.getBook(path.c_str());
    bookTitles[i] = RECENT_BOOKS.getDataFromBook(path).title;
    if (bookTitles[i].empty()) bookTitles[i] = path;
    char minutesBuf[24];
    char valueBuf[64];
    snprintf(valueBuf, sizeof(valueBuf), "%s · %lu",
             minutesText(stats ? stats->totalMinutes : 0, minutesBuf, sizeof(minutesBuf)),
             static_cast<unsigned long>(stats ? stats->sessions : 0));
    bookValues[i] = valueBuf;
    bookItems[i].label = bookTitles[i].c_str();
    bookItems[i].value = bookValues[i].c_str();
  }

  fui::ListProps props;
  props.items = bookItems.data();
  props.count = static_cast<uint16_t>(bookItems.size());
  props.action = ACTION_ROW;
  props.inputMask = fui::InputTouch;
  props.valueInset = 8;
  props.labelText = screen.theme().smallText;
  props.labelText.maxLines = 2;
  syncTabListViewport(screen, props);
  screen.list(props);
}

void ReadingStatsActivity::buildHeatmap(UiScreen& screen) {
  const fui::Rect body = screen.body();
  auto& target = screen.target();
  // Paged geometry: HEATMAP_PAGE_WEEKS columns per page, so cells stay large
  // enough to read. heatPage 0 is the oldest page; the newest page ends on
  // the current week's Saturday.
  const int labelColWidth = 16;  // weekday letter column
  const int monthRowHeight = target.lineHeight(fui::GfxRendererTarget::FONT_SMALL) + 4;
  const int availableWidth = body.width - labelColWidth - 4;
  const int gap = 4;
  int cell = (availableWidth - (HEATMAP_PAGE_WEEKS - 1) * gap) / HEATMAP_PAGE_WEEKS;
  const int maxCellByHeight = (body.height - monthRowHeight - (HEATMAP_ROWS - 1) * gap) / HEATMAP_ROWS;
  if (cell > maxCellByHeight) cell = maxCellByHeight;
  if (cell < 3) cell = 3;
  const int pageWeeks = HEATMAP_PAGE_WEEKS;
  const int gridWidth = pageWeeks * cell + (pageWeeks - 1) * gap;
  const int gridHeight = HEATMAP_ROWS * cell + (HEATMAP_ROWS - 1) * gap;
  const int originX = body.x + labelColWidth + (availableWidth - gridWidth) / 2;
  const int originY = body.y + monthRowHeight + (body.height - monthRowHeight - gridHeight) / 2;

  const int32_t today = ReadingStats::todayDayNumber();
  const int todayWeekday = ReadingStats::weekdayOfDay(today);
  const int32_t fullGridStart = today - todayWeekday - (HEATMAP_WEEKS - 1) * 7;
  const int32_t pageStartDay = fullGridStart + heatPage * pageWeeks * 7;
  const int32_t gridStart = pageStartDay;

  const fui::Paint ink = fui::Paint::solid(fui::Color::Black);
  const fui::Paint paper = fui::Paint::solid(fui::Color::White);

  static constexpr const char* kMonthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  fui::TextStyle monthStyle = screen.theme().smallText;
  const int gridRight = originX + gridWidth;
  int lastMonth = -1;
  // Label the first week of each month, plus the trailing month's label when
  // the page ends mid-month (e.g. a page spanning Mar 28 – Jun 27 shows Apr,
  // May, Jun — and "Jul" if Jul 1-4 fall inside the last week).
  for (int week = 0; week < pageWeeks; week++) {
    int y;
    unsigned mo, d;
    ReadingStats::civilFromDays(gridStart + week * 7, y, mo, d);
    // The label goes on the week containing the 1st of the month, or the
    // first week of the page when the month started before it.
    const bool isNewMonth = static_cast<int>(mo) != lastMonth;
    const bool isTrailing = week == pageWeeks - 1 && d < 7;  // last week crosses into next month
    if (!isNewMonth && !isTrailing) continue;
    lastMonth = static_cast<int>(mo);
    const char* name = kMonthNames[mo - 1];
    const int labelWidth = target.measureText(monthStyle.font, name, monthStyle).width + 2;
    int lx = originX + week * (cell + gap);
    if (lx + labelWidth > gridRight) lx = gridRight - labelWidth;  // clamp inside the grid
    target.text(fui::Rect{static_cast<int16_t>(lx), static_cast<int16_t>(originY - monthRowHeight),
                          static_cast<int16_t>(labelWidth), static_cast<int16_t>(monthRowHeight)},
                name, monthStyle);
  }
  // Trailing-month label: if the page's last day is in a new month, show it.
  {
    int y;
    unsigned mo, d;
    ReadingStats::civilFromDays(gridStart + pageWeeks * 7 - 1, y, mo, d);
    if (static_cast<int>(mo) != lastMonth) {
      const char* name = kMonthNames[mo - 1];
      const int labelWidth = target.measureText(monthStyle.font, name, monthStyle).width + 2;
      target.text(
          fui::Rect{static_cast<int16_t>(gridRight - labelWidth), static_cast<int16_t>(originY - monthRowHeight),
                    static_cast<int16_t>(labelWidth), static_cast<int16_t>(monthRowHeight)},
          name, monthStyle);
    }
  }

  // Page indicator "oldest .. newest" sits under the grid, right-aligned —
  // above would collide with the month labels.
  {
    char pageBuf[32];
    char d0[11];
    char d1[11];
    ReadingStats::formatDayKey(gridStart, d0, sizeof(d0));
    ReadingStats::formatDayKey(gridStart + pageWeeks * 7 - 1, d1, sizeof(d1));
    snprintf(pageBuf, sizeof(pageBuf), "%s .. %s", d0 + 2, d1 + 2);  // strip century
    fui::TextStyle pageStyle = monthStyle;
    pageStyle.align = fui::TextAlign::Right;
    const int pw = target.measureText(pageStyle.font, pageBuf, pageStyle).width + 2;
    target.text(
        fui::Rect{static_cast<int16_t>(body.x + body.width - pw), static_cast<int16_t>(originY + gridHeight + gap),
                  static_cast<int16_t>(pw), static_cast<int16_t>(monthRowHeight)},
        pageBuf, pageStyle);
  }
  // Weekday letters: only when the cell is tall enough to fit the glyph.
  // Column width is measured from the widest letter (M/W), not hardcoded —
  // 16px clipped them to ".." at the small font size.
  if (cell >= 10) {
    const int dayLabelWidth = target.measureText(monthStyle.font, "W", monthStyle).width + 4;
    static constexpr const char* kDayLetters[] = {"S", "M", "T", "W", "T", "F", "S"};
    for (int row = 1; row < HEATMAP_ROWS; row += 2) {
      target.text(fui::Rect{static_cast<int16_t>(body.x), static_cast<int16_t>(originY + row * (cell + gap)),
                            static_cast<int16_t>(dayLabelWidth), static_cast<int16_t>(cell)},
                  kDayLetters[row], monthStyle);
    }
  }

  for (int week = 0; week < pageWeeks; week++) {
    for (int row = 0; row < HEATMAP_ROWS; row++) {
      const int32_t day = gridStart + week * 7 + row;
      const int x = originX + week * (cell + gap);
      const int y = originY + row * (cell + gap);
      const fui::Rect cellRect{static_cast<int16_t>(x), static_cast<int16_t>(y), static_cast<int16_t>(cell),
                               static_cast<int16_t>(cell)};
      const bool isCursor = week == heatCursorWeek && row == heatCursorRow;
      const bool future = day > today;

      int level = 0;
      if (!future) {
        if (const auto* stats = STATS_STORE.getDay(day)) {
          level = ReadingStats::intensityLevel(stats->minutes);
        }
      }

      switch (level) {
        case 1:
          target.fill(cellRect, fui::Paint::dither(fui::Color::LightGray));
          break;
        case 2:
          target.fill(cellRect, fui::Paint::dither(fui::Color::DarkGray));
          break;
        case 3:
          target.fill(cellRect, fui::Paint::dither(fui::Color::DarkGray));
          // Second pass: darken with a checkerboard of solid pixels.
          for (int py = y; py < y + cell; py += 2) {
            for (int px = x + (py - y) % 2; px < x + cell; px += 2) {
              target.line(fui::Point{static_cast<int16_t>(px), static_cast<int16_t>(py)},
                          fui::Point{static_cast<int16_t>(px), static_cast<int16_t>(py)}, 1, ink);
            }
          }
          break;
        case 4:
          target.fill(cellRect, ink);
          break;
        default:
          target.stroke(cellRect, ink, 1);
          break;
      }

      if (isCursor) {
        // Inverted frame marks the focused cell (visible over every fill).
        target.stroke(fui::Rect{static_cast<int16_t>(x - 1), static_cast<int16_t>(y - 1),
                                static_cast<int16_t>(cell + 2), static_cast<int16_t>(cell + 2)},
                      ink, 1);
        if (level == 4) {
          target.stroke(fui::Rect{static_cast<int16_t>(x), static_cast<int16_t>(y), static_cast<int16_t>(cell),
                                  static_cast<int16_t>(cell)},
                        paper, 1);
        }
      }
    }
  }
}

void ReadingStatsActivity::openBook(const int index) {
  if (index < 0 || index >= static_cast<int>(statBookPaths.size())) return;
  app.clearTapFlash();
  onSelectBook(statBookPaths[index]);
}

void ReadingStatsActivity::showBookDetail(const int index) {
  if (index < 0 || index >= static_cast<int>(statBookPaths.size())) return;
  const auto& path = statBookPaths[index];
  const auto* stats = STATS_STORE.getBook(path.c_str());

  std::string title = RECENT_BOOKS.getDataFromBook(path).title;
  if (title.empty()) title = path;

  char totalBuf[24];
  char avgBuf[24];
  char line0[96];
  char line1[96];
  char line2[96];
  const uint32_t totalMin = stats ? stats->totalMinutes : 0;
  const uint32_t activeDays = stats ? stats->activeDays : 0;
  const uint32_t sessions = stats ? stats->sessions : 0;
  const uint32_t completed = stats ? stats->completed : 0;
  snprintf(line0, sizeof(line0), "%s: %s", tr(STR_STATS_TOTAL_LABEL),
           minutesText(totalMin, totalBuf, sizeof(totalBuf)));
  snprintf(line1, sizeof(line1), "%s: %lu · %s: %lu", tr(STR_STATS_ACTIVE_DAYS), static_cast<unsigned long>(activeDays),
           tr(STR_STATS_SESSIONS_LABEL), static_cast<unsigned long>(sessions));
  snprintf(line2, sizeof(line2), "%s: %s · %s: %lu", tr(STR_STATS_MIN_PER_DAY),
           minutesText(activeDays > 0 ? totalMin / activeDays : 0, avgBuf, sizeof(avgBuf)),
           tr(STR_STATS_COMPLETED_LABEL), static_cast<unsigned long>(completed));

  // Strings must outlive the popup: OptionPopup::show copies them.
  const char* options[] = {line0, line1, line2, tr(STR_STATS_OPEN_BOOK)};
  const int detailIndex = index;
  bookPopup.show(title.c_str(), options, 4, 0, [this, detailIndex](int sel) {
    if (sel == 3) openBook(detailIndex);
  });
}

void ReadingStatsActivity::showDayDetail() {
  const int32_t today = ReadingStats::todayDayNumber();
  const int32_t fullGridStart = today - ReadingStats::weekdayOfDay(today) - (HEATMAP_WEEKS - 1) * 7;
  const int32_t day = fullGridStart + (heatPage * HEATMAP_PAGE_WEEKS + heatCursorWeek) * 7 + heatCursorRow;
  if (day > today) return;  // no data in the future

  char dateBuf[11];
  ReadingStats::formatDayKey(day, dateBuf, sizeof(dateBuf));
  char minutesBuf[24];
  char detail[64];
  const auto* stats = STATS_STORE.getDay(day);
  const uint32_t minutes = stats ? stats->minutes : 0;
  const uint32_t sessions = stats ? stats->sessions : 0;
  snprintf(detail, sizeof(detail), "%s: %s, %lu", dateBuf, minutesText(minutes, minutesBuf, sizeof(minutesBuf)),
           static_cast<unsigned long>(sessions));
  const char* options[] = {detail};
  dayPopup.show(dateBuf, options, 1, 0, [](int) {});
}

void ReadingStatsActivity::activateIndex(const int index) {
  if (selectedTab == 1) {
    showBookDetail(index);
  } else if (selectedTab == 2) {
    showDayDetail();
  }
}

void ReadingStatsActivity::onTabAction(const int index) {
  selectedTab = index;
  activeNav().selected = 0;
  activeNav().followOnBuild = true;
  app.clearTapFlash();
  requestUpdate();
}

void ReadingStatsActivity::stepTab(const int direction) {
  const bool onTabBar = ringPos() == 0;
  selectedTab = direction > 0 ? ButtonNavigator::nextIndex(selectedTab, tabCount())
                              : ButtonNavigator::previousIndex(selectedTab, tabCount());
  activeNav().selected = onTabBar ? 0 : 1;
  activeNav().followOnBuild = true;
  requestUpdate();
}

bool ReadingStatsActivity::handleCustomInput() {
  if (dayPopup.handleInput(mappedInput, [this] { requestUpdate(); })) return true;
  if (bookPopup.handleInput(mappedInput, [this] { requestUpdate(); })) return true;
  return false;
}

void ReadingStatsActivity::navigateButtons() {
  // Same ring walk as UiTabListActivity; popup opens on Confirm, not ring move.
  const int ringSize = listCount() + 1;
  buttonNavigator.onNextRelease([this, ringSize] { moveRingTo(ButtonNavigator::nextIndex(ringPos(), ringSize)); });
  buttonNavigator.onPreviousRelease(
      [this, ringSize] { moveRingTo(ButtonNavigator::previousIndex(ringPos(), ringSize)); });
  buttonNavigator.onNextContinuous([this] { stepTab(1); });
  buttonNavigator.onPreviousContinuous([this] { stepTab(-1); });
}

bool ReadingStatsActivity::handleButtons() {
  // Heatmap: directional buttons move the cell cursor / flip pages while the
  // ring is on the grid element.
  if (selectedTab == 2 && ringPos() == 1 && !dayPopup.isActive() && !bookPopup.isActive()) {
    constexpr int kLastPage = (HEATMAP_WEEKS - 1) / HEATMAP_PAGE_WEEKS;
    bool moved = false;
    if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
      if (heatCursorWeek > 0) {
        heatCursorWeek--;
      } else if (heatPage > 0) {
        heatPage--;
        heatCursorWeek = HEATMAP_PAGE_WEEKS - 1;
      }
      moved = true;
    } else if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
      if (heatCursorWeek < HEATMAP_PAGE_WEEKS - 1) {
        heatCursorWeek++;
      } else if (heatPage < kLastPage) {
        heatPage++;
        heatCursorWeek = 0;
      }
      moved = true;
    } else if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
      heatCursorRow = (heatCursorRow + HEATMAP_ROWS - 1) % HEATMAP_ROWS;
      moved = true;
    } else if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
      heatCursorRow = (heatCursorRow + 1) % HEATMAP_ROWS;
      moved = true;
    }
    if (moved) {
      requestUpdate();
      return true;
    }
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (ringPos() == 0) {
      stepTab(1);
    } else if (selectedTab == 2) {
      showDayDetail();
      requestUpdate();
    } else if (selectedTab == 1) {
      showBookDetail(ringPos() - 1);
      requestUpdate();
    }
    return true;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    if (ringPos() > 0) {
      activeNav().selected = 0;
      requestUpdate();
    } else {
      onGoHome();
    }
    return true;
  }

  return false;
}

void ReadingStatsActivity::drawFooter() {
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void ReadingStatsActivity::render(RenderLock&& lock) {
  if (dayPopup.processRender(renderer, mappedInput)) return;
  if (bookPopup.processRender(renderer, mappedInput)) return;
  UiTabListActivity::render(std::move(lock));
}

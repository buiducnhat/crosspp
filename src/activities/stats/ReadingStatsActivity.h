#pragma once

#include <ReadingStatsStore.h>

#include <cstdint>
#include <string>
#include <vector>

#include "activities/UiTabListActivity.h"
#include "components/OptionPopup.h"

// Reading statistics screen: Settings-style tab band (UiTabListActivity
// chrome) over four tabs — Overview (tile grid of totals + habit tiles),
// By Book (list of Recent Books with per-book stats; Confirm opens a
// book-detail popup, which can jump into the book), Heatmap (52 weeks paged
// ~13 weeks/page so cells stay readable; Left/Right flips pages, Up/Down
// moves the weekday cursor, Confirm shows the day's detail popup), Insights
// (passive list of reading-habit metrics computed from the daily aggregate).
class ReadingStatsActivity final : public UiTabListActivity {
  // --- tab-shared state ---
  int selectedTab = 0;

  // --- By Book ---
  std::vector<std::string> statBookPaths;  // Recent Books paths that have stats
  std::vector<freeink::ui::ListItem> bookItems;
  std::vector<std::string> bookTitles;
  std::vector<std::string> bookValues;
  OptionPopup bookPopup;
  int lastAutoPopupRow = -1;  // ring row that last auto-opened the popup; -1 = none

  // --- Heatmap ---
  static constexpr int HEATMAP_WEEKS = 52;
  static constexpr int HEATMAP_ROWS = 7;
  static constexpr int HEATMAP_PAGE_WEEKS = 13;
  int heatPage = 3;        // oldest page = 0; 3 shows the current week
  int heatCursorRow = 0;   // weekday of the focused day
  int heatCursorWeek = 0;  // week within the visible page
  OptionPopup dayPopup;

  // --- Insights ---
  // Value buffers are members: ListItem pointers must stay valid until the
  // next buildScreen pass.
  static constexpr int INSIGHT_ROWS = 7;
  static constexpr size_t INSIGHT_VALUE_SIZE = 32;
  freeink::ui::ListItem insightItems[INSIGHT_ROWS];
  const char* insightLabels[INSIGHT_ROWS] = {};
  char insightValues[INSIGHT_ROWS][INSIGHT_VALUE_SIZE] = {};

  // --- UiTabListActivity contract ---
  int listCount() const override;
  int tabCount() const override { return 4; }
  int activeTab() const override { return selectedTab; }
  const char* tabLabel(int index) const override;
  void buildScreen(UiScreen& screen) override;
  void activateIndex(int index) override;
  void onTabAction(int index) override;
  void stepTab(int direction) override;
  bool handleButtons() override;
  bool handleCustomInput() override;
  void navigateButtons() override;

  const char* headerTitle() const override;
  void drawFooter() override;

  void showBookDetail(int index);
  void buildOverview(UiScreen& screen);
  void buildInsights(UiScreen& screen);
  void buildByBook(UiScreen& screen);
  void buildHeatmap(UiScreen& screen);
  void rebuildBookItems();
  void openBook(int index);
  void showDayDetail();
  static const char* minutesText(uint32_t minutes, char* buf, size_t bufSize);

 public:
  explicit ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void render(RenderLock&&) override;
};

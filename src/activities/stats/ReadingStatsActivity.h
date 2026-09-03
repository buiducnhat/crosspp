#pragma once

#include <ReadingStatsStore.h>

#include <cstdint>
#include <string>
#include <vector>

#include "activities/UiTabListActivity.h"
#include "components/OptionPopup.h"

// Reading statistics screen: Settings-style tab band (UiTabListActivity
// chrome) over four tabs — Overview (passive list of reading totals and
// habit metrics), By Book (list of Recent Books with per-book stats; Confirm
// opens a book-detail popup, which can jump into the book), Heatmap (52 weeks
// paged ~13 weeks/page so cells stay readable; Left/Right flips pages, Up/Down
// moves the weekday cursor, Confirm shows the day's detail popup), Settings
// (clock synchronization and time preferences).
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

  // --- Overview ---
  // Value buffers are members: ListItem pointers must stay valid until the
  // next buildScreen pass.
  static constexpr int OVERVIEW_BASE_ROWS = 10;
  static constexpr int OVERVIEW_MAX_ROWS = 11;
  static constexpr size_t OVERVIEW_VALUE_SIZE = 48;
  freeink::ui::ListItem overviewItems[OVERVIEW_MAX_ROWS];
  const char* overviewLabels[OVERVIEW_MAX_ROWS] = {};
  char overviewValues[OVERVIEW_MAX_ROWS][OVERVIEW_VALUE_SIZE] = {};

  // --- Settings ---
  static constexpr int STATS_SETTINGS_ROWS = 4;
  freeink::ui::ListItem settingsItems[STATS_SETTINGS_ROWS];
  char settingsValues[STATS_SETTINGS_ROWS][64] = {};
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
  int overviewCount() const;
  void buildOverview(UiScreen& screen);
  void buildByBook(UiScreen& screen);
  void buildHeatmap(UiScreen& screen);
  void buildSettings(UiScreen& screen);
  void handleSettingsSelection(int index);
  void rebuildBookItems();
  void openBook(int index);
  void showDayDetail();
  static const char* minutesText(uint32_t minutes, char* buf, size_t bufSize);

 public:
  explicit ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  void onEnter() override;
  void render(RenderLock&&) override;
};

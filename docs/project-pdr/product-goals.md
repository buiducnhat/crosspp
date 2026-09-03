# Product Goals, Features and Design Decisions

## Product Vision

CrossPP is a focused, distraction-free e-reader firmware optimized specifically for the Xteink X3/X4 and compatible ESP32 e-paper hardware. The primary objective is to deliver a fast, reliable, battery-efficient reading experience without excess bloat that compromises memory stability.

---

## Core Product Capabilities

### 1. Offline & Local-First Reading Experience
- **Broad Format Support**: Fast rendering for `.epub` (EPUB 2 and 3), `.txt`, `.bmp`, and `.xtc/.xtch` comic formats.
- **Embedded vs Overriding Styles**: Gives users the choice between book-defined CSS layout or clean uniform font and margin rules.
- **StarDict Dictionary Support**: Instant word lookup using local dictionary files placed on the SD card (`/dict/`).
- **Reading Progress Sync**: Compatible with KOReader sync server to share progress between mobile devices and CrossPP.

### 2. Built-in Reading Statistics (CrossPP Highlight)
- **Local & Private**: All metrics persist strictly to SD (`/.crosspoint/reading_stats.json`). No telemetry or network connections needed.
- **Automatic Session Measurement**: Runs in the background with an idle timeout (5 minutes) so paused reading does not inflate stats.
- **Four Dedicated Analytical and Management Tabs**:
  1. **Overview**: Unified list view showing reading milestones and habit insights (total reading time, books completed, sessions, avg session length, current/longest streaks, 7-day/30-day daily averages, best day, top reading day, and clock calibration status if unsynced).
  2. **By Book**: Reading progress, time spent, active days, and sessions per book with direct launch capability.
  3. **Heatmap**: 52-week activity calendar rendered in 13-week quarters with 4-level dithering for quick habit visualization.
  4. **Settings**: On-screen clock synchronization (NTP with startup delay absorption across multiple reliable servers), UTC timezone offset adjustment, 12-hour/24-hour format toggle, and live current time verification.
### 3. Wireless Management Without Cables
- **Embedded Web Manager**: Connect to the reader via Wi-Fi hotspot (AP mode) or home network (STA mode) to upload books, organize files, and edit settings.
- **WebDAV Support**: Mount the e-reader as a network drive on desktop OSes.
- **OPDS Catalog Browser**: Directly browse and download books from Calibre Content Server, Standard Ebooks, and Project Gutenberg.
- **OTA Updates**: Download and apply firmware updates directly from GitHub releases.

---

## Technical & Product Non-Goals
- **No Heavy Browser**: No full web browsing engine; network access is strictly for book transfers, OPDS catalogs, and sync.
- **No Background Audio / Media Player**: Audio would exhaust CPU and RAM buffers essential for page layout stability.
- **No Bloated Cloud Dependencies**: The firmware remains fully functional without Wi-Fi or any online accounts.

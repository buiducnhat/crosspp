# Codebase Directory Structure and Key Modules

## Source Tree Overview

```text
crosspp/
├── lib/                     # Reusable firmware engines and third-party libraries
│   ├── EpdFont/             # E-ink font engine (compressed Flash fonts & SD-card .cpfont)
│   ├── Epub/                # EPUB archive reader, HTML parser, and layout engine
│   ├── GfxRenderer/         # 1-bit frame buffer renderer, shapes, text, dither primitives
│   ├── hal/                 # Hardware Abstraction Layer (Display, GPIO, Storage)
│   ├── I18n/                # Localization engine and translations (YAML source)
│   ├── KOReaderSync/        # KOReader progress sync protocol
│   ├── Logging/             # Core logging macros (LOG_INF, LOG_DBG, LOG_ERR)
│   ├── Memory/              # RAII allocation utilities (makeUniqueNoThrow)
│   ├── OpdsParser/          # XML/Atom OPDS catalog parser
│   ├── ReadingStats/        # Reading session tracker, JSON persistence, habit insights
│   ├── Txt/                 # Plain text file rendering engine
│   └── Xtc/                 # XTC/XTCH manga format reader
├── src/                     # Application source code
│   ├── activities/          # UI screen implementations following Activity lifecycle
│   │   ├── boot_sleep/      # Boot screen, low-battery warning, sleep & lock screens
│   │   ├── browser/         # File browser (SD card navigation)
│   │   ├── home/            # Main launcher menu, cover grid, continue reading
│   │   ├── network/         # Web server, wireless file transfer, AP/STA setup
│   │   ├── reader/          # Primary book reader activity (EPUB, TXT, XTC)
│   │   ├── settings/        # Device configuration, theme, buttons, font downloader
│   │   ├── stats/           # ReadingStatsActivity (Overview, By Book, Heatmap, Settings)
│   │   └── util/            # Dialogs, keyboard, frontlight panel, battery viewer
│   ├── components/          # UI Theme, dialog cards, option popups, nav bars
│   ├── network/             # Embedded HTTP/WebDAV server, OTA updater, captive portal
│   ├── CrossPointSettings.h # Persistent user preferences (JSON backed)
│   ├── CrossPointState.h    # Transient runtime state (current book, sleep states)
│   ├── MappedInputManager.h # Logical button and touch mapping
│   ├── RecentBooksStore.h   # Recently opened books registry
│   └── main.cpp             # Hardware setup, global fonts, main FreeRTOS loop
├── data/                    # HTML and Web UI static assets compiled at build time
├── scripts/                 # Build-time code generators (i18n, HTML headers, font tools)
└── platformio.ini           # Build flags, partition layouts, and environment definitions
```

## Key Modules & Responsibilities

| Module | Location | Responsibilities |
| --- | --- | --- |
| **ActivityManager** | `src/activities/ActivityManager.h` | Top-level screen navigation, stack switching, lifecycle control (`onEnter`, `loop`, `onExit`). |
| **ReaderActivity** | `src/activities/reader/ReaderActivity.h` | Rendering pages, handling turns, font size changing, bookmarks, footnotes, StarDict dictionary lookup, and triggering `ReadingSessionTracker`. |
| **ReadingStatsActivity** | `src/activities/stats/ReadingStatsActivity.h` | 4-tab statistics interface: Overview (scrollable metrics list), By Book (list with detail popup), Heatmap (52-week calendar), Settings (NTP sync & clock configuration). |
| **GfxRenderer** | `lib/GfxRenderer/GfxRenderer.h` | Direct drawing into the 48KB 1-bit buffer: lines, circles, boxes, dithered fills, text layouts, and coordinate transformations across 4 orientations. |
| **HalStorage** | `lib/hal/HalStorage.h` | Singleton `Storage` providing mutex-synchronized `HalFile` handles over SdFat to prevent SPI task race conditions. |
| **CrossPointSettings** | `src/CrossPointSettings.h` | Stores orientation, font choice, layout margins, button behaviors, and theme preferences to `/.crosspoint/settings.json`. |
| **UITheme** | `src/components/UITheme.h` | Singleton `GUI` providing design tokens, standard margins, headers, footers, button hints, and popup framing. |

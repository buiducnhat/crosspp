# CrossPP (crosspp)

[![Release](https://img.shields.io/github/v/release/buiducnhat/crosspp?style=for-the-badge&color=blue)](https://github.com/buiducnhat/crosspp/releases)
[![License](https://img.shields.io/badge/license-GPLv3-green?style=for-the-badge)](./LICENSE)

**CrossPP** (`crosspp`) is a lightweight, high-performance open-source e-reader firmware based on [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader), tailored for the ESP32-C3 based Xteink [X3](https://www.xteink.com/products/xteink-x3) / [X4](https://www.xteink.com/products/xteink-x4) and adjacent ESP32 e-paper hardware.

It introduces **Reading Statistics** directly into the reading experience alongside fast EPUB rendering, custom fonts, offline dictionaries, wireless transfers, and customizable themes.

### Now running on:
- **ESP32C3-based** Xteink X4 and X3.
- **ESP32S3-based** Xteink X4Pro, Seeed reTerminal Sticky, M5PaperMono

![CrossPP Reader running on Xteink device](./docs/images/cover.jpg)

---

## What makes CrossPP special?

### 📊 Built-in Reading Statistics
CrossPP tracks your reading habits locally and privately on your SD card without needing external accounts:

- **Active Session Tracking**: Automatically logs reading sessions in `ReaderActivity` with a 5-minute idle timeout so paused time isn't counted.
- **Home Menu Entry**: Directly accessible from the main menu with a dedicated bar-chart icon.
- **Overview Tab**: Unified, scrollable list combining reading milestones (total reading time, books completed, sessions, avg session length) with habit insights (current/longest streaks, 7-day/30-day daily averages, best reading day, top reading day) and an instant clock-sync alert if the RTC has not been set.
- **By Book Tab**: Lists all recently read books with per-book reading time and session counts. Selecting any book opens an in-depth statistics modal (Total time, Active days, Sessions, Minutes/day, Completed) with an option to open the book directly.
- **Activity Heatmap Tab**: GitHub-style activity calendar with 4 intensity levels (up to ≥60 min/day):
  - Paginated by quarter (~13 weeks per view) for large, legible cells.
  - Month and weekday (Mon, Wed, Fri) labels.
  - 4-way D-pad cursor navigation.
  - Day-detail popup displaying exact date, active minutes, and sessions.
- **Settings Tab**: On-screen clock synchronization (robust NTP sync across multiple servers with startup-delay absorption), UTC timezone offset adjustment, 12h/24h toggle, and live current time verification.
---

## Features

- **Reader Engine**: Fast EPUB 2/3 rendering with embedded-style option, background section indexing, image handling, hyphenation, kerning, adaptive table layouts, native CJK ruby annotations, chapter navigation, footnotes, bookmarks, dictionary lookups ([StarDict](docs/dictionary.md)), go-to-percent, auto page turn, orientation control, focus reading, and KOReader progress sync.
- **Supported Formats**: Native support for `.epub`, `.xtc/.xtch`, `.txt`, and `.bmp`.
- **Custom Fonts**: Install your favorite TTF/OTF fonts on the SD card as `.cpfont` files.
- **Touch reading**: follow EPUB links and look up words in the dictionary on touch-enabled devices.
- **Screenshots.**
- **Tilt page turn (X3 and Sticky)**: Accelerometer-driven page turns.
- **USB Drive mode (X4Pro)**: access the SD card as USB mass storage.
- **Library workflow**: folder browser, hidden-file toggle, long-press delete, recent books, SD-cache management.
- **Wireless Workflows**:
  - Web UI for file transfer and book management (`http://crosspp.local/`)
  - EPUB Optimizer
  - Web settings UI/API to configure device settings from your browser
  - WebSocket fast uploads and WebDAV support
  - AP mode (hotspot) and STA mode (Wi-Fi) with on-screen QR codes
  - Calibre wireless connect flow
  - OPDS browser with saved servers, search, pagination, and direct download
  - OTA update checks and installs from GitHub releases
- **Customization**: night mode, multiple themes (Classic, Lyra, Lyra Extended, RoundedRaff), sleep screen modes including transparent overlays, front/side button remapping, status bar controls, power-button behavior, refresh cadence, and more.
- **Localization**: 34 UI languages and counting, including CJK font fallback and RTL support (Arabic, Hebrew, Farsi, Urdu).

### Coming soon:

- More themes.
- Web plugins.
- Bluetooth pageturner.
- Much more! stay tuned.

---

## Installation

### Option 1: Web Installer (Recommended)

1. Connect your device to your computer via USB-C and wake/unlock it.
2. Go to [crosspointreader.com/#flash-tools](https://crosspointreader.com/#flash-tools), select your device model (X3 or X4).
3. Click **"Custom .bin"** and upload the `firmware.bin` downloaded from [CrossPP Releases](https://github.com/buiducnhat/crosspp/releases).

### Option 2: Command Line (`esptool`)

> ### ⚠️ WARNING: READ THIS BEFORE USING THE UNLOCKER ⚠️
>
> **The only officially supported firmwares in the unlock tool are CrossPoint and CrossInk.**
>
> Flashing any other firmware on a USB-locked device may **permanently brick the device** or leave it **permanently
> stuck on that firmware with no recovery path**. Once USB flashing is re-locked, your only way back is via OTA, and if
> the firmware you flashed doesn't support OTA, **there is no way out**.

1. Install [`esptool`](https://github.com/espressif/esptool):

```bash
pip install esptool
```

2. Download the firmware file for your device from [CrossPP Releases](https://github.com/buiducnhat/crosspp/releases), or compile yourself.
3. Connect your device via USB-C and locate the serial port (`/dev/ttyACM0` on Linux, `/dev/cu.usbmodem*` on macOS).
4. Flash an X3 or X4:

```bash
esptool.py --chip esp32c3 --port /dev/ttyACM0 --baud 921600 write_flash 0x10000 /path/to/firmware.bin
```

   Flash an Xteink X4Pro, Seeed reTerminal Sticky, or M5PaperMono:

```bash
esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 921600 write_flash 0x10000 /path/to/firmware.bin
```

### Revert to Official Firmware

To revert to the official firmware, you can also flash the latest official firmware using https://crosspointreader.com/#flash-tools.

---

## USB-locked Devices (Xteink Unlocker)

Some Xteink units purchased from third-party stores (e.g., AliExpress) ship with factory-locked USB flashing. If your device is locked, use the **Xteink Unlocker** tool at https://crosspointreader.com/#unlock-tool before flashing. Units purchased directly from xteink.com are not locked.

---

## Custom SD-card Fonts

Convert your own TTF/OTF files into `.cpfont` files that load from the SD card without reflashing firmware:

1. Visit [crosspointreader.com/fonts](https://crosspointreader.com/fonts) and open the font builder form.
2. Upload font styles, set family name and point sizes, and download the `.cpfont` files.
3. Copy them to your SD card under `/fonts/YourFont/` (or `/.fonts/YourFont/`).
4. Select the font in the device settings.

---

## Development Quick Start

### Prerequisites

- [pioarduino](https://github.com/pioarduino/pioarduino) or VS Code + PlatformIO/pioarduino extension
- Python 3.8+
- `clang-format` 21+
- USB-C data cable

### Clone & Build

```bash
git clone --recursive https://github.com/buiducnhat/crosspp.git
cd crosspp

# If cloned without --recursive:
git submodule update --init --recursive
```

### Build firmware:

```bash
# Default development build (serial logging enabled)
pio run

# Build release binary for Xteink X3/X4
pio run -e gh_release

# Build and upload to connected device
pio run -t upload

# Run desktop simulator (if configured in platformio.local.ini)
pio run -e simulator_x3
```

### Code Quality

```bash
./bin/clang-format-fix -g
pio check
```

### Nix/NixOS

Nix/NixOS users can enter the development shell with either `nix develop` (flakes) or `nix-shell`:

```bash
nix develop -f nix
# or
nix-shell nix
```

To flash a connected ESP32-C3 device, enable PlatformIO's udev rules in your NixOS configuration:

```nix
services.udev.packages = with pkgs; [ platformio-core.udev ];
```

After rebuilding the system configuration, reconnect the device or reload udev rules.

### Debugging

After flashing the new features, it’s recommended to capture detailed logs from the serial port.

First, make sure all required Python packages are installed:

```python
python3 -m pip install pyserial colorama matplotlib
```

After that run the script:

```sh
# For Linux
# This was tested on Debian and should work on most Linux systems.
python3 scripts/debugging_monitor.py

# For macOS
python3 scripts/debugging_monitor.py /dev/cu.usbmodem2101
```

Minor adjustments may be required for Windows.

---

## Documentation

- **[Documentation Index & Developer Guide](./docs/SUMMARY.md)** ([Bản tiếng Việt](./docs-vi/SUMMARY.md))
- [User Guide](./USER_GUIDE.md)
- [Web Server Usage](./docs/webserver.md)
- [Web Server Endpoints](./docs/webserver-endpoints.md)
- [Dictionary Setup](./docs/dictionary.md)
- [Touch & UI Development](./docs/contributing/touch-and-ui.md)
- [File Formats & Cache](./docs/file-formats.md)

---

## Internals

CrossPP inherits CrossPoint's aggressive SD-card caching to minimise RAM usage. The ESP32-C3 only has ~380KB of usable RAM, so many firmware design decisions follow from this constraint.

### Data caching

The first time chapters of a book are loaded, they are cached to the SD card. Subsequent loads are served from the
cache. This cache directory exists at `.crosspoint` on the SD card. The structure is as follows:

```text
.crosspoint/
├── epub_<hash>/         # one directory per book, named by content hash
│   ├── progress.bin     # reading position (chapter, page, etc.)
│   ├── cover.bmp        # generated cover image
│   ├── book.bin         # metadata: title, author, spine, TOC
│   ├── css_rules.cache  # parsed CSS rule cache
│   ├── img_*            # rendered image cache files
│   └── sections/        # per-chapter layout cache
│       ├── 0.bin
│       ├── 1.bin
│       └── ...
├── settings.json        # device settings
├── state.json           # resume/runtime state
└── recent.json          # recent books list
```

Removing `/.crosspoint` clears all cached metadata and forces a full regeneration on next open. Book deletes, overwrites, and moves done through the firmware or web UI clear or re-key matching caches; manual SD-card edits may leave stale cache directories behind.

For more details on the internal file structures, see the [file formats document](./docs/file-formats.md).

---

## Contributing

Contributions are welcome. If you're new to the codebase, start with the [contributing docs](./docs/contributing/README.md). For things to work on, check the [ideas discussion board](https://github.com/crosspoint-reader/crosspoint-reader/discussions/categories/ideas) — leave a comment before starting so we don't duplicate effort.

Everyone here is a volunteer, so please be respectful and patient. For governance and community expectations, see [GOVERNANCE.md](./GOVERNANCE.md).

---

## Community forks

One of the best things about open source is that anyone can take the code in a different direction. If you need something outside CrossPoint's [scope](./SCOPE.md), check out the community forks:

- [CrossInk](https://github.com/uxjulia/CrossInk) — UX focused with minimal reading stats and broader customizations for the reading experience.

- [papyrix-reader](https://github.com/bigbag/papyrix-reader) — Adds FB2 and MD format support. Actively maintained with Arabic script support. Custom themes.

- [inx](https://github.com/obijuankenobiii/inx) — Completely reimagines the user interface with tabbed navigation.

- [Witch(hunt) Reader](https://github.com/jpirnay/witchhunt-reader) — More faithful CSS styling and background work for slightly snappier interaction. Weather information panel. Markdown support.

**Note:** Many of these features will make their way into CrossPoint over time. Each project chooses its own priorities and tradeoffs.

Want to build your own device? Be sure to check out the [de-link](https://github.com/iandchasse/de-link) project or [OnePage Reader](https://github.com/MoveCall/onepage-reader).

---

## Credits & Acknowledgments

- Based on the excellent [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) project by [crosspoint-reader](https://github.com/crosspoint-reader) and community contributors.
- Low-level drivers and graphics powered by [FreeInk SDK](https://freeink.org).
- Inspired by [diy-esp32-epub-reader](https://github.com/atomic14/diy-esp32-epub-reader).
- CrossPP is an independent open-source fork and is **not affiliated with Xteink or any device manufacturer**.

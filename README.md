# CrossPP (crosspp)

[![Release](https://img.shields.io/github/v/release/buiducnhat/crosspp?style=for-the-badge&color=blue)](https://github.com/buiducnhat/crosspp/releases)
[![License](https://img.shields.io/badge/license-GPLv3-green?style=for-the-badge)](./LICENSE)

**CrossPP** (`crosspp`) is a lightweight, high-performance open-source e-reader firmware based on [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader), tailored for the ESP32-C3 based Xteink [X3](https://www.xteink.com/products/xteink-x3) / [X4](https://www.xteink.com/products/xteink-x4) and adjacent ESP32 e-paper hardware.

It introduces **Reading Statistics** directly into the reading experience alongside fast EPUB rendering, custom fonts, offline dictionaries, wireless transfers, and customizable themes.

![CrossPP Reader running on Xteink device](./docs/images/cover.jpg)

---

## What makes CrossPP special?

### 📊 Built-in Reading Statistics
CrossPP tracks your reading habits locally and privately on your SD card without needing external accounts:

- **Active Session Tracking**: Automatically logs reading sessions in `ReaderActivity` with a 5-minute idle timeout so paused time isn't counted.
- **Home Menu Entry**: Directly accessible from the main menu with a dedicated bar-chart icon.
- **Overview Tab**: Highlights reading milestones (total reading time in hours/minutes, session count, books completed) rendered in a clean card grid.
- **By Book Tab**: Lists all recently read books with per-book reading time and session counts. Selecting any book opens an in-depth statistics modal (Total time, Active days, Sessions, Minutes/day, Completed) with an option to open the book directly.
- **Activity Heatmap**: GitHub-style activity calendar with 4 intensity levels (up to ≥60 min/day):
  - Paginated by quarter (~13 weeks per view) for large, legible cells.
  - Month and weekday (Mon, Wed, Fri) labels.
  - 4-way D-pad cursor navigation.
  - Day-detail popup displaying exact date, active minutes, and sessions.

---

## Features

- **Reader Engine**: Fast EPUB 2/3 rendering with embedded-style option, background section indexing, image handling, hyphenation, kerning, chapter navigation, footnotes, bookmarks, dictionary lookups ([StarDict](docs/dictionary.md)), go-to-percent, auto page turn, orientation control, focus reading, and KOReader progress sync.
- **Supported Formats**: Native support for `.epub`, `.xtc/.xtch`, `.txt`, and `.bmp`.
- **Custom Fonts**: Install your favorite TTF/OTF fonts on the SD card as `.cpfont` files.
- **Tilt Page Turn**: Accelerometer-driven page turns (X3).
- **Wireless Workflows**:
  - Web UI for file transfer and book management (`http://crosspp.local/`)
  - EPUB Optimizer
  - Web settings UI/API to configure device settings from your browser
  - WebSocket fast uploads and WebDAV support
  - AP mode (hotspot) and STA mode (Wi-Fi) with on-screen QR codes
  - Calibre wireless connect flow
  - OPDS browser with saved servers, search, pagination, and direct download
  - OTA update checks and installs from GitHub releases
- **Customization**: Multiple themes (Classic, Lyra, Lyra Extended, RoundedRaff), transparent sleep screen overlays, button remapping, status bar options, and refresh controls.
- **Localization**: 32 UI languages with RTL support (Arabic, Hebrew, Farsi, Urdu).

---

## Installation

### Option 1: Web Installer (Recommended)

1. Connect your device to your computer via USB-C and wake/unlock it.
2. Go to [crosspointreader.com/#flash-tools](https://crosspointreader.com/#flash-tools), select your device model (X3 or X4).
3. Click **"Custom .bin"** and upload the `firmware.bin` downloaded from [CrossPP Releases](https://github.com/buiducnhat/crosspp/releases).

### Option 2: Command Line (`esptool`)

1. Install `esptool`:
   ```bash
   pip install esptool
   ```
2. Download `firmware.bin` from [CrossPP Releases](https://github.com/buiducnhat/crosspp/releases).
3. Connect your device via USB-C and locate the serial port (`/dev/ttyACM0` on Linux, `/dev/cu.usbmodem*` on macOS).
4. Flash:
   ```bash
   esptool.py --chip esp32c3 --port /dev/ttyACM0 --baud 921600 write_flash 0x10000 firmware.bin
   ```

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

---

## Documentation

- [User Guide](./USER_GUIDE.md)
- [Web Server Usage](./docs/webserver.md)
- [Web Server Endpoints](./docs/webserver-endpoints.md)
- [Dictionary Setup](./docs/dictionary.md)
- [Touch & UI Development](./docs/contributing/touch-and-ui.md)
- [File Formats & Cache](./docs/file-formats.md)

---

## Credits & Acknowledgments

- Based on the excellent [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) project by [crosspoint-reader](https://github.com/crosspoint-reader) and community contributors.
- Low-level drivers and graphics powered by [FreeInk SDK](https://freeink.org).
- Inspired by [diy-esp32-epub-reader](https://github.com/atomic14/diy-esp32-epub-reader).
- CrossPP is an independent open-source fork and is **not affiliated with Xteink or any device manufacturer**.

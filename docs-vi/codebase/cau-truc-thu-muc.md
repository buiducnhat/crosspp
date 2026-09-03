# Cấu trúc codebase

## Entry point

- `src/main.cpp` — boot, init HAL/settings/fonts, vòng lặp chính, điều hướng activity (heap-allocate + delete khi chuyển), load toàn bộ font toàn cục (`#ifndef OMIT_FONTS`).
- `src/fontIds.h` — ID font dùng với `renderer.insertFont()` / `drawText()`.

## Bản đồ thư mục

| Đường dẫn | Vai trò |
| --- | --- |
| `src/` | Orchestration ứng dụng, settings/state, các store, activities |
| `src/activities/` | Controller màn hình theo nhóm: `home/`, `reader/`, `settings/`, `network/`, `browser/`, `boot_sleep/`, `util/`; base ở `Activity.h`, `UiListActivity.h`, `ActivityManager.*` |
| `src/components/` | `UITheme` (macro `GUI`), themes trong `themes/`, icons, dialog dùng chung |
| `src/network/` | `CrossPointWebServer` (HTTP/WebSocket/WebDAV/UDP), `OtaUpdater`, `OtaBootSwitch`, `FirmwareFlasher`, `FirmwareBoardTag`, HTML đã generate trong `html/` |
| `src/util/` | Tiện ích: URL, QR, screenshot, string, book cache, bookmark… |
| `lib/hal/` | HAL bọc SDK: `HalDisplay`, `HalGPIO`, `HalStorage` (singleton `Storage`), `HalFrontlight`, `HalTiltSensor`, `HalSystem`, `HalClock` |
| `lib/Epub/` | Engine EPUB: parse OPF/TOC/CSS, layout section, hyphenation, cache `book.bin`/`section.bin` |
| `lib/Xtc/`, `lib/Txt/` | Reader cho định dạng XTC/XTCH và TXT/MD |
| `lib/GfxRenderer/` | Renderer framebuffer, bitmap, font cache (`GfxRenderer.cpp` ~90KB là file lớn nhất) |
| `lib/EpdFont/` | Font engine: `EpdFont`, `EpdFontFamily`, font nén + giải nén, font SD (`.cpfont`), `builtinFonts/` |
| `lib/I18n/` | `tr()` / `I18N`; nguồn dịch `translations/*.yaml` → generate `I18nKeys.h`, `I18nStrings.*` |
| `lib/Serialization/` | `PersistableStore` (JSON lên SD qua HalStorage), `BufferedFile`, obfuscation |
| `lib/KOReaderSync/` | Đồng bộ tiến độ KOReader: client, credential store, map XPath ↔ offset |
| `lib/ZipFile/`, `lib/miniz/`, `lib/uzlib/`, `lib/InflateReader/` | Giải nén ZIP/deflate phục vụ EPUB |
| `lib/expat/` | XML parser (giới hạn `XML_CONTEXT_BYTES=1024`, tắt general entities `XML_GE=0`) |
| `lib/PngToBmpConverter/`, `lib/JpegToBmpConverter/` | Convert ảnh sách/bìa sang BMP cho e-ink |
| `lib/MiniBidi/`, `lib/Utf8/` | RTL/bidi và xử lý UTF-8 |
| `lib/Memory/` | `makeUniqueNoThrow` (bắt buộc thay bare `new`), scratch buffer |
| `lib/Logging/` | `LOG_INF` / `LOG_DBG` / `LOG_ERR` |
| `lib/OpdsParser/`, `lib/JsonParser/` | OPDS catalog, streaming JSON |
| `freeink-sdk/` | **Git submodule** (driver display, input, SD, battery, board profiles). `platformio.ini` link vào qua `symlink://freeink-sdk/libs/...`. Docs: https://freeink.org/llms.txt |
| `scripts/` | Pre/post build: `build_html.py`, `gen_i18n.py`, `git_branch.py` (version), patch wolfSSL/JPEGDEC, sinh EPUB test |
| `data/html/` | Nguồn HTML web UI → generate ra `src/network/html/*.generated.h` |
| `test/` | Unit test (target đăng ký bởi `register_unit_tests_target.py`) |

## Module trung tâm

- `src/CrossPointSettings.h` (`SETTINGS`) — mọi preference; persist `/.crosspoint/settings.json`.
- `src/CrossPointState.h` (`APP_STATE`) — trạng thái phiên, resume sách; `/.crosspoint/state.json`.
- `src/MappedInputManager.*` — nút logic → nút vật lý theo settings/orientation.
- `src/SdCardFontSystem.*`, `src/FontInstaller.*` — font SD (`.cpfont`) và cài font qua web.
- `src/RecentBooksStore.*`, `src/OpdsServerStore.*`, `src/WifiCredentialStore.*` — các `PersistableStore` JSON.
- `src/SettingsList.h` — định nghĩa toàn bộ mục cài đặt UI (~28KB).

## Luồng khởi động file

Boot → `main.cpp` → HAL init → settings/state → fonts → `HomeActivity` hoặc resume `ReaderActivity` → dispatch theo extension sang `EpubReaderActivity` / `XtcReaderActivity` / `TxtReaderActivity`.

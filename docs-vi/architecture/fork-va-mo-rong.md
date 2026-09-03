# Fork firmware mới hoặc mở rộng CrossPoint

Phân tích các điểm neo (touchpoint) khi: (A) fork thành firmware độc lập, (B) port sang board mới, (C) mở rộng tính năng trên base hiện có. Mọi đường dẫn đã kiểm chứng trên nhánh `develop`.

## A. Fork thành firmware riêng

Checklist tối thiểu để fork không "dính" về dự án gốc:

### 1. OTA update — rủi ro lớn nhất

`src/network/OtaUpdater.cpp:22` hardcode:

```cpp
constexpr char latestReleaseUrl[] = "https://api.github.com/repos/crosspoint-reader/crosspoint-reader/releases/latest";
```

**Nếu không đổi, thiết bị chạy fork sẽ tự OTA về firmware CrossPoint gốc.** Đổi sang repo fork của bạn. Quy ước tên asset (`OtaUpdater.cpp:34-38`): `firmware.bin` cho binary C3 X4/X3, `firmware-<board>.bin` cho các board khác → workflow release của fork phải xuất đúng tên asset này (xem `.github/workflows/release.yml`).

### 2. Version

- Base version: `platformio.ini` → `[crosspoint] version = 1.5.0`.
- Env release ghép version qua `-DCROSSPOINT_VERSION=\"${crosspoint.version}\"`; env dev (`default`, `sticky`) do `scripts/git_branch.py` sinh (version + branch + short SHA).
- Fork nên đổi version để phân biệt trên màn hình/boot log.

### 3. Board tag (chống flash nhầm board)

`src/network/FirmwareBoardTag.h`: mỗi image nhúng tag `CROSSPOINT-BOARD-V1:<board>;`. Đường OTA/flash từ chối image mang tag board **khác**, nhưng **cho phép image không có tag** (fork của bên thứ ba). Hệ quả:

- Fork giữ nguyên cơ chế này vẫn được bảo vệ chéo giữa các board trong fork.
- Firmware gốc sẽ không chặn image fork (và ngược lại) — người dùng tự chịu trách nhiệm khi flash chéo qua esptool.

### 4. Partition layout

`partitions.csv`: OTA kép `app0`/`app1` (0x640000 mỗi cái, offset 0x10000), `otadata`, `spiffs`, `coredump`. OTA boot-switch (`src/network/OtaBootSwitch.cpp`) phụ thuộc layout này — **không đổi** trừ khi viết lại cả đường OTA. Binary build ra flash ở offset `0x10000`.

### 5. Xung đột cache SD `/.crosspoint/`

Đường dẫn `/.crosspoint` hardcode rải rác, không phải một hằng số duy nhất: `CrossPointSettings.h:380`, `CrossPointState.h:27`, `OpdsServerStore.h:31`, `RecentBooksStore.h:29`, `WifiCredentialStore.h:44`, `src/main.cpp:221`, và literal `"/.crosspoint"` truyền vào constructor `Epub`/`Xtc`/`Txt` ở nhiều nơi (`HomeActivity.cpp:68`, `SleepActivity.cpp:771-799`, các ReaderActivity…).

- Fork **giữ nguyên format cache** → dùng chung `/.crosspoint` an toàn; hai firmware có thể ghi đè cache của nhau nhưng format-version guard tự invalidate.
- Fork **thay format cache** → bump `SECTION_FILE_VERSION` (`lib/Epub/Epub/Section.cpp`) và version `book.bin` (`BookMetadataCache.cpp`), hoặc đổi tên thư mục gốc để cô lập hoàn toàn (phải sửa mọi điểm trên). Quy tắc format: `docs/file-formats.md`.

### 6. Kênh phát hành

- Web flasher `crosspointreader.com` là hạ tầng dự án gốc — fork tự host flasher hoặc hướng dẫn esptool (`esptool.py --chip esp32c3 write_flash 0x10000 firmware.bin`).
- CI: `.github/workflows/` (`ci.yml`, `release.yml`, `release_candidate.yml`, `pr-formatting-check.yml`) — chỉnh tên artifact/board cho khớp mục 1.

### 7. Pháp lý/nhận diện

Đổi tên hiển thị, logo (`src/images/Logo120.h`), README. Kiểm tra license của repo và các thư viện kèm theo trước khi phát hành.

## B. Port sang board mới

Board support nằm ở 2 lớp: **env trong `platformio.ini`** + **BoardConfig profile trong freeink-sdk** (submodule). Mẫu tham khảo: env `sticky`, `x4pro`, `papermono`.

### 1. Thêm env trong `platformio.ini`

```ini
[env:myboard]
extends = base
board = esp32-s3-devkitc1-n16r8   ; hoặc esp32-c3-devkitm-1 cho C3
board_build.mcu = esp32s3
build_flags =
  ${base.build_flags}
  -DFREEINK_DEVICE_MYBOARD=1      ; định danh board, SDK đọc flag này
  -DBOARD_HAS_PSRAM               ; chỉ khi có PSRAM
  -DCROSSPOINT_VERSION=\"${crosspoint.version}-myboard\"
  -DUSE_BLOCK_DEVICE_INTERFACE=1  ; SD native SDMMC thay vì SPI
  -DFREEINK_FRONTLIGHT_LS         ; frontlight sống qua light sleep
```

Một binary per MCU family (C3 khác S3). Flag ví dụ thực tế: X4 Pro dùng `USE_BLOCK_DEVICE_INTERFACE` (SD 1-bit SDMMC) + `FREEINK_FRONTLIGHT_LS`; Paper Mono dùng PSRAM cho grayscale planes.

### 2. Board profile trong freeink-sdk

Pinout, display driver, touch (GT911/FT6336), frontlight, gauge, cảm biến nghiêng… do profile trong `freeink-sdk/libs/hardware/BoardConfig` quyết định theo `FREEINK_DEVICE_*`. SDK là submodule riêng → fork cần fork cả SDK hoặc upstream profile. Kiểm tra API khả dụng cho target trước tại https://freeink.org/llms.txt. Board tag (mục A.3) tự gắn theo tên board, chặn OTA nhầm giữa các board S3 cùng chip_id.

### 3. Điểm cần kiểm chứng trên phần cứng mới

- Framebuffer 800x480 1-bit = 48.000B đặt trong DRAM (C3) hoặc PSRAM batching (S3 grayscale).
- Orientation: mọi UI phải qua `renderer.getScreenWidth()/getScreenHeight()` và `getOrientedViewableTRBL()` — cấm hardcode 800/480.
- Nút bấm: map qua `MappedInputManager`, không dùng raw `HalGPIO::BTN_*`.
- Nếu SD dùng SDMMC block-device, xác nhận `HalStorage` vẫn serialize qua `storageMutex`.

## C. Mở rộng trên base (không fork)

### 1. Thêm màn hình mới

Kế thừa `Activity` (`src/activities/Activity.h`) hoặc base UI có sẵn (`UiListActivity`, `UiTabListActivity`, FreeInkUI — xem `docs/contributing/touch-and-ui.md`). Vòng đời: `onEnter()` cấp phát → `loop()` xử lý input qua `mappedInput` → `onExit()` giải phóng (task FreeRTOS phải `vTaskDelete()` trước khi object bị `delete`). Điều hướng qua `ActivityManager` (`docs/activity-manager.md`). Render qua macro `GUI` (UITheme), text qua `tr(STR_*)`.

### 2. Thêm định dạng sách mới

Dispatch tập trung tại `src/activities/reader/ReaderActivity.cpp:31-37` theo extension (`FsHelpers::has*Extension`). Cần chạm:

1. Parser lib mới trong `lib/` (mẫu: `lib/Txt/` nhỏ nhất, `lib/Xtc/` có cache).
2. Reader activity mới (mẫu: `TxtReaderActivity`).
3. Extension helper trong `lib/FsHelpers/`.
4. Cover/metadata ở `HomeActivity.cpp:67-90`, `RecentBooksStore.cpp:126-135`.
5. Sleep screen cover: `SleepActivity.cpp:771-799`.
6. Dọn cache: `src/util/BookCacheUtils.cpp:24-30`.

### 3. Thêm ngôn ngữ UI

Thêm `lib/I18n/translations/<lang>.yaml` (đủ `_language_name`, `_language_code`, `_order`, `_bcp47`, các key `STR_*`), chạy `python scripts/gen_i18n.py lib/I18n/translations lib/I18n/`. File generate ra đã gitignore. Chi tiết: `docs/i18n.md`, `docs/translators.md`.

### 4. Thêm setting mới

Field trong `src/CrossPointSettings.h` (+ `toJson`/`fromJson`), entry UI trong `src/SettingsList.h`. Nếu setting ảnh hưởng layout sách, thêm vào tham số cache-busting của section cache (xem `docs/file-formats.md`) và nhớ guard ghi SD thừa (PersistableStore debounce).

### 5. Theme

Hệ theme **đang đóng băng** theo `SCOPE.md` mục 3 — PR theme mới bị đóng cho tới khi themes chuyển sang load từ SD. Muốn làm theme: tham gia loạt việc "move themes off-firmware" thay vì thêm theme mới.

### 6. Vùng cấm scope

`SCOPE.md`: không app tương tác (notepad/game), không công cụ soạn thảo, không RSS/web browser, không PDF, tạm đóng connector mạng mới. Ý tưởng ngoài core reading → nên làm fork thay vì PR.

## D. Ràng buộc chung khi mở rộng

- Trần RAM 380KB (C3) là ràng buộc thiết kế đầu tiên; mọi heap allocation phải qua `makeUniqueNoThrow` + justify.
- Thay format binary cache → bump version file trước khi đổi struct (`docs/file-formats.md`).
- Mọi SD I/O qua `HalStorage`; cấm gọi SdFat trực tiếp (mutex, tránh panic `xTaskPriorityDisinherit`).
- Workflow đóng góp: nhánh `feature/` `fix/` …, PR nhắm `develop`, qua `./bin/clang-format-fix -g` + `pio check` + `pio run -e default` trước khi review.

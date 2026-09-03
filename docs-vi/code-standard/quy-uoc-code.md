# Quy ước code

Chuẩn C++20 (`-std=gnu++2a`), **không exceptions, không RTTI** (`-fno-exceptions`). Nguồn đầy đủ: `AGENTS.md` gốc — file này là bản tóm tắt vận hành.

## Đặt tên và tổ chức

- Class `PascalCase` (`EpubReaderActivity`); method/biến `camelCase`; hằng `UPPER_SNAKE_CASE`; member private không prefix; tên file trùng tên class.
- Header guard: `#pragma once`.
- Comment: ngắn, viết cho trạng thái đã merge; bỏ narration trước/sau, số đo điều tra (thuộc về commit message).

## Bộ nhớ (ràng buộc 380KB RAM)

- **Cấm bare `new`**: với `-fno-exceptions`, OOM gọi `abort()`. Dùng `makeUniqueNoThrow<T>()` từ `lib/Memory/Memory.h`; luôn null-check + `LOG_ERR` trước khi `return false`. `new (std::nothrow)` trực tiếp chỉ khi C API nhận ownership.
- Biến local < 256B (stack C3 nhỏ); buffer lớn → heap/static, cấp phát một lần trong `onEnter()` và tái dùng.
- `std::vector`: luôn `.reserve(N)` trước vòng `push_back`.
- Hằng số/bảng tra: `constexpr` (hoặc `static constexpr` trong class) để nằm trong flash, không tốn DRAM.
- Cấm `std::string`/Arduino `String` trong hot path: dùng `std::string_view` (đọc) và `snprintf` + `char[]` cố định (dựng).
- Tránh `std::function` và template bloat trong code path render.

## Cạm bẫy nền tảng

- `std::string_view` **không null-terminated**: cấm `.data()` vào C API; convert `std::string(view).c_str()` hoặc `snprintf("%.*s", ...)`.
- ISR: `IRAM_ATTR`; dữ liệu ISR đọc: `DRAM_ATTR`. Mutex không gọi được trong ISR — dùng `xQueueSendFromISR`/`xSemaphoreGiveFromISR`.
- RISC-V fault khi load unaligned: cấm cast `uint8_t*` → kiểu rộng rồi dereference; dùng `memcpy`.
- E-paper/logic UI: cấm hardcode 800/480 — dùng `renderer.getScreenWidth()/getScreenHeight()` và `getOrientedViewableTRBL()`; kiểm tra đủ 4 orientation.

## HAL và SD

- Luôn dùng HAL (`HalDisplay`, `HalGPIO`, `HalStorage` singleton `Storage`), cấm gọi SDK/SdFat trực tiếp. SdFat không thread-safe; mọi SD I/O qua `HalFile` (mutex `storageMutex`).
- `DESTRUCTOR_CLOSES_FILE=1`: **không** gọi `file.close()` cho `FsFile`/`HalFile` local. Ngoại lệ: close trước khi `Storage.remove()` cùng path, close trước khi reopen cùng biến, và member file handle (close trong `onExit()`).
- Nút bấm: dùng enum logic `MappedInputManager::Button::*`, cấm raw `HalGPIO::BTN_*` (trừ `ButtonRemapActivity`).

## Logging và lỗi

- `LOG_INF` / `LOG_DBG` / `LOG_ERR` từ `Logging.h`; cấm `Serial.print` trực tiếp.
- Phân cấp: `LOG_ERR + return false` (mặc định) → `LOG_ERR + fallback` → `assert(false)` chỉ cho trạng thái "không thể" → `ESP.restart()` chỉ cho recovery OTA.

## UI và i18n

- Mọi render UI qua macro `GUI` (UITheme); cấm hardcode font/màu/vị trí.
- Text người dùng thấy: bắt buộc `tr(STR_*)`; log message được hardcode.

## Build flags định hình hành vi

`EINK_DISPLAY_SINGLE_BUFFER_MODE=1` (1 framebuffer 48KB — grayscale cần `storeBwBuffer()`/`restoreBwBuffer()`), `XML_CONTEXT_BYTES=1024`, `XML_GE=0`, `USE_UTF8_LONG_NAMES=1`, `MINIZ_NO_ZLIB_COMPATIBLE_NAMES=1`, `ARDUINO_USB_CDC_ON_BOOT=1`. Xem đầy đủ và chú thích MEMFIX-PORT (wolfSSL SP ECC, `custom_sdkconfig` reclaim ~32–37KB heap) trong `platformio.ini`.

## File generate — không sửa tay

`src/network/html/*.generated.h` (sửa `data/html/`), `lib/I18n/I18nKeys.h`/`I18nStrings.*` (sửa `lib/I18n/translations/*.yaml` rồi chạy `scripts/gen_i18n.py`). Chỉ commit nguồn, không commit file generate.

## Vòng đời Activity

`onEnter()` cấp phát → `loop()` → `onExit()` giải phóng; activity bị `delete` khi điều hướng sang activity khác. Task FreeRTOS: `xTaskCreate` stack tính bằng **byte** (2048 render đơn giản, 4096 network/EPUB parse), luôn `vTaskDelete()` trong `onExit()` trước khi object hủy.

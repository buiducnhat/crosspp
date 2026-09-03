# Quy trình phát triển

## Môi trường build

PlatformIO (extension VS Code `platformio.platformio-ide` hoặc CLI `pio`), Python 3.8+, `clang-format` 21 (chỉ gọi qua wrapper `./bin/clang-format-fix`, không gọi trực tiếp). Nix: `nix develop -f nix` hoặc `nix-shell nix`.

```bash
git clone --recursive https://github.com/crosspoint-reader/crosspoint-reader
cd crosspoint-reader
git submodule update --init --recursive   # nếu clone thiếu --recursive
```

Lưu ý: `freeink-sdk` là submodule — build thất bại kiểu "missing library" thường do chưa init submodule.

## Các môi trường `platformio.ini`

| Env | MCU | Mục đích |
| --- | --- | --- |
| `default` | ESP32-C3 (X4/X3) | Dev, `LOG_LEVEL=2`, chờ USB serial |
| `gh_release` / `gh_release_rc` | C3 | Production / RC (`LOG_LEVEL=1`) |
| `slim` | C3 | Tối giản, tắt serial log |
| `sticky` (+ `-gh_release`, `-gh_release_rc`) | ESP32-S3 | Seeed Sticky, touch GT911 |
| `x4pro` (+ variants) | ESP32-S3 + PSRAM | Xteink X4 Pro, SDMMC, frontlight light-sleep |
| `papermono` (+ variants) | ESP32-S3 + PSRAM | M5Stack Paper Mono |

`platformio.local.ini` (gitignored) cho override cá nhân: `upload_port`, `monitor_port`, flag debug riêng. **Không bao giờ commit.**
## Simulator desktop (không cần thiết bị)

[crosspoint-simulator](https://github.com/crosspoint-reader/crosspoint-simulator) compile firmware native và render e-ink vào cửa sổ SDL2. Cần `brew install sdl2` (macOS) hoặc `libsdl2-dev libssl-dev` (Linux/WSL; Windows native không hỗ trợ).

Repo này đã cấu hình sẵn `[env:simulator]` trong `platformio.local.ini`. Sách đặt ở `./fs_/books/` (map thành `/books/` trên SD ảo; `fs_/` giữ vai trò thẻ SD).

```bash
pio run -e simulator -t run_simulator
```

Phím: ↑/↓ lật trang, ←/→ nút trái/phải, Return xác nhận, Escape quay lại, P nguồn, S giả lập sleep, H phím Home (X4 Pro), chuột = tap/swipe (board touch). Các env khác: `simulator_x3`, `simulator_x4_pro`, `simulator_sticky`, `simulator_papermono` (xem `sample-platformio-macos.ini` trong repo simulator).

QA tự động không cần GUI: `CROSSPOINT_SIM_INPUT_SCRIPT="4000:ENTER;20000:QUIT"` và `CROSSPOINT_SIM_SCREENSHOTS="3500:/tmp/home.bmp"` rồi chạy `.pio/build/simulator/program`.

Lưu ý khi firmware đổi API HAL: stub trong `.pio/libdeps/simulator/simulator/src/` phải sửa cho khớp (ví dụ chữ ký `HalGPIO::verifyPowerButtonWakeup()`). Bản vá trong `libdeps` mất khi xóa thư mục này — fix lâu dài phải upstream vào repo simulator.

## Lệnh thường dùng

```bash
pio run                      # build env default
pio run -t upload            # build + flash
pio run -e x4pro -t upload   # build + flash board khác
pio check -e default         # cppcheck
./bin/clang-format-fix -g    # format các file C/C++ đã sửa trong git
python3 scripts/debugging_monitor.py /dev/cu.usbmodem2101   # monitor có màu (macOS)
```

Build đầu tiên chậm do rebuild Arduino core (vì `custom_sdkconfig`). Nếu bị lỗi `multiple definition of 'app_main'` sau build ngắt giữa chừng:

```bash
rm -rf .dummy CMakeLists.txt sdkconfig.default sdkconfig.defaults .pio/build/default
```

Không dùng `git clean -fdX` — mất `platformio.local.ini`.

## Checklist trước PR

1. `./bin/clang-format-fix -g`
2. `pio check -e default`
3. `pio run -e default` (build một lần sau edit cuối)
4. CI xanh: `.github/workflows/ci.yml` (build) + `pr-formatting-check.yml` (format)
5. Rà soát orientation: đủ 4 mode Portrait/Inverted/Landscape CW/CCW trong switch/case

Phần kiểm chứng trên máy thật (heap `ESP.getFreeHeap()` > 50KB, 4 orientation, xóa `/.crosspoint/` nếu động vào EPUB) thuộc về người test.

## Git workflow

- Luôn kiểm tra `git remote -v` đầu phiên: repo có thể là fork (`origin` cá nhân + `upstream` gốc) hoặc clone trực tiếp.
- PR và branch tích hợp nhắm **`develop`**, không phải `master`.
- Không push/đóng/mở PR khi chưa được duyệt; nếu được duyệt push, ưu tiên remote `fork` cho feature branch.
- Nhánh: `feature/<mô-tả>`, `fix/<issue>-<mô-tả>`, `refactor/<component>`, `docs/<chủ-đề>`.
- Commit: `<type>: <summary ≤50 chars>` với type `feat|fix|refactor|docs|test|chore|perf`.
- Khi adapt PR của người khác: giữ `Co-Authored-By` tác giả gốc (bỏ bot); không thêm trailer attribution cho AI.
- Trước khi stage: `git status` đối chiếu `.gitignore` (`*.generated.h`, `.pio/`, `compile_commands.json`, `platformio.local.ini` không được stage).

## Debug crash thường gặp

- OOM: log `ESP.getFreeHeap()` quanh nghiệp vụ; kiểm tra alloc >10KB và giải phóng trong `onExit()`.
- Stack overflow: `uxTaskGetStackHighWaterMark()` < 512B → tăng stack 2048→4096 hoặc chuyển buffer lên heap.
- Use-after-free: task còn sống sau khi activity bị delete → luôn `vTaskDelete()` trước.
- Cache hỏng: xóa `/.crosspoint/` trên SD để ép re-parse; version format trong `docs/file-formats.md`.
- Watchdog: vòng lặp nghẹt >5s → thêm `vTaskDelay(1)`, kiểm tra I/O blocking.

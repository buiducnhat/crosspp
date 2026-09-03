# Mục tiêu sản phẩm và ràng buộc

Tóm lược từ `SCOPE.md` (nguồn chuẩn) và ràng buộc kỹ thuật trong codebase.

## Sứ mệnh

Firmware đọc sách nhẹ, hiệu năng cao cho e-reader ESP32 (khởi đầu là Xteink X3/X4, đang mở rộng sang hệ sinh thái e-ink ESP32 nhỏ: X4 Pro, Seeed Sticky, M5Stack Paper Mono). Nguyên tắc: làm **một việc** thật tốt — trải nghiệm đọc tập trung — thay vì firmware "đồ nghề". Code device-specific phải nằm sau ranh giới HAL/SDK để lõi reader portable.

Nguyên tắc định hướng khi đánh giá tính năng mới: lấp khoảng trống firmware gốc làm kém; nếu một fork phổ biến đã giải tốt vấn đề ngoài lõi đọc thì nhường cho fork đó; không cải thiện trải nghiệm đọc hoặc khả năng bảo trì → ngoài scope.

## Use case chính

- Đọc EPUB 2/3 (CSS, ảnh, hyphenation, kerning, footnote, bảng), XTC/XTCH, TXT/MD, xem BMP.
- Điều hướng: mục lục, bookmark, go-to-percent, auto page turn, 4 orientation, focus reading, dictionary StarDict, đồng bộ tiến độ KOReader.
- Thư viện: duyệt thư mục, sách gần đây, ẩn file, xóa long-press, quản lý cache SD.
- Không dây: web UI truyền file, WebDAV, WebSocket upload, OPDS (tối đa 8 server), Calibre wireless, AP/STA kèm QR, OTA từ GitHub Releases.
- Tùy biến: theme (đang freeze), font SD `.cpfont`, remap nút, status bar, sleep screen.
- Đa ngôn ngữ: 24+ ngôn ngữ UI, hỗ trợ RTL.

## Trạng thái scope hiện tại

Trong scope: render/typography EPUB, driver e-ink (giảm ghosting), UX đọc, quản lý thư viện, giảm footprint RAM/flash, refactor.

Tạm đóng: theme mới; connector mạng mới (sync engine, cloud client, mở rộng OPDS).

Ngoài scope: app tương tác (notepad, game), công cụ soạn thảo, RSS/trình duyệt, render PDF.

## Ràng buộc cứng

| Ràng buộc | Giá trị | Hệ quả thiết kế |
| --- | --- | --- |
| RAM dùng được (C3) | ~380KB, không PSRAM | Cache SD-first, allocation kiểm soát chặt, trần thiết kế cho mọi tính năng |
| Framebuffer | 1 × 48.000B (800×480÷8), single-buffer | Grayscale cần buffer tạm qua `storeBwBuffer()` |
| Flash | 16MB, OTA kép 2×6.4MB | `custom_sdkconfig` reclaim heap, tối ưu binary size |
| CPU | C3 single-core RISC-V 160MHz; S3 dual-core Xtensa | Vòng lặp chính phải responsive, watchdog 5s |
| Màn hình | E-ink 800×480 mono, refresh 1–2s | Batch render, chính sách refresh tiết kiệm |
| Lưu trữ | SD (SPI hoặc SDMMC 1-bit) | Mọi I/O qua `HalStorage` mutex; SPIFFS không mount |

## Quyết định kiến trúc đáng nhớ

- **Cache SD-first**: parse/layout persist vào `/.crosspoint/epub_<hash>/` để mở lại không reparse; đổi render settings (font, margin, orientation…) invalidate section cache.
- **Tiến độ theo content-offset**: section cache chứa bảng offset → vị trí đọc bền qua đổi font/orientation, map được với XPath KOReader.
- **Board tag trong image**: OTA từ chối flash nhầm board cùng chip family (`FirmwareBoardTag.h`).
- **wolfSSL TLS 1.3 với SP ECC single-precision**: ECC P-256 chạy trên mảng 256-bit cố định thay vì bignum heap-alloc — tránh OOM ở ~50KB heap còn lại của phiên đọc.

## Tài trợ và quản trị

Contributor được trả qua Royalty.dev theo impact/tenure (minh bạch tại app.royalty.dev/transparency). Governance: `GOVERNANCE.md`. Fork cộng đồng được khuyến khích cho hướng đi ngoài scope — danh sách fork trong `README.md`.

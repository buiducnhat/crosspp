# Cấu trúc mã nguồn và các module chính

## Tổng quan cây thư mục

```text
crosspp/
├── lib/                     # Các engine tái sử dụng và thư viện bên thứ ba
│   ├── EpdFont/             # Engine font E-ink (font nén trong Flash & định dạng .cpfont trên thẻ SD)
│   ├── Epub/                # Trình đọc file nén EPUB, phân tích HTML và dàn trang
│   ├── GfxRenderer/         # Render bộ đệm khung hình 1-bit, hình học, text, thuật toán dither
│   ├── hal/                 # Lớp trừu tượng phần cứng (Màn hình, GPIO, Bộ nhớ thẻ SD)
│   ├── I18n/                # Hệ thống đa ngôn ngữ và các bản dịch (nguồn YAML)
│   ├── KOReaderSync/        # Giao thức đồng bộ tiến độ đọc với KOReader
│   ├── Logging/             # Macro ghi nhật ký hệ thống (LOG_INF, LOG_DBG, LOG_ERR)
│   ├── Memory/              # Tiện ích cấp phát an toàn RAII (makeUniqueNoThrow)
│   ├── OpdsParser/          # Trình phân tích danh mục sách trực tuyến OPDS (XML/Atom)
│   ├── ReadingStats/        # Theo dõi phiên đọc, lưu trữ JSON, phân tích thói quen đọc sách
│   ├── Txt/                 # Engine hiển thị file văn bản thuần (.txt)
│   └── Xtc/                 # Trình đọc truyện tranh định dạng XTC/XTCH
├── src/                     # Mã nguồn ứng dụng chính
│   ├── activities/          # Cài đặt các màn hình UI theo vòng đời Activity
│   │   ├── boot_sleep/      # Màn hình khởi động, cảnh báo pin yếu, màn hình ngủ & khóa
│   │   ├── browser/         # Trình duyệt tệp tin trên thẻ SD
│   │   ├── home/            # Màn hình chính, lưới bìa sách, đọc tiếp sách gần nhất
│   │   ├── network/         # Máy chủ web, truyền tệp không dây, cài đặt Wi-Fi AP/STA
│   │   ├── reader/          # Trình đọc sách chính (EPUB, TXT, XTC)
│   │   ├── settings/        # Cài đặt thiết bị, chủ đề (theme), nút bấm, tải font chữ
│   │   ├── stats/           # ReadingStatsActivity (Tổng quan, Theo sách, Heatmap, Thói quen)
│   │   └── util/            # Hộp thoại, bàn phím ảo, bảng chỉnh đèn nền, xem dung lượng pin
│   ├── components/          # Giao diện UITheme, thẻ thông báo, popup lựa chọn, thanh điều hướng
│   ├── network/             # Máy chủ HTTP/WebDAV nhúng, cập nhật OTA, captive portal
│   ├── CrossPointSettings.h # Cài đặt người dùng lưu trữ lâu dài (qua JSON)
│   ├── CrossPointState.h    # Trạng thái runtime tạm thời (sách đang mở, trạng thái màn hình ngủ)
│   ├── MappedInputManager.h # Ánh xạ phím bấm vật lý và cử chỉ chạm thành nút logic
│   ├── RecentBooksStore.h   # Danh sách các cuốn sách đọc gần đây
│   └── main.cpp             # Khởi tạo phần cứng, font tĩnh toàn cục, vòng lặp FreeRTOS chính
├── data/                    # Tài nguyên tĩnh Web UI biên dịch vào firmware lúc build
├── scripts/                 # Kịch bản sinh mã lúc biên dịch (i18n, header HTML, công cụ font)
└── platformio.ini           # Cấu hình cờ biên dịch, phân vùng bộ nhớ và môi trường PlatformIO
```

## Các module cốt lõi & Trách nhiệm

| Module | Đường dẫn | Trách nhiệm |
| --- | --- | --- |
| **ActivityManager** | `src/activities/ActivityManager.h` | Điều hướng màn hình cấp cao nhất, chuyển đổi stack màn hình, quản lý vòng đời (`onEnter`, `loop`, `onExit`). |
| **ReaderActivity** | `src/activities/reader/ReaderActivity.h` | Render nội dung sách, xử lý lật trang, thay đổi cỡ chữ, dấu trang, chú thích cuối trang (footnote), tra từ điển StarDict, và kích hoạt `ReadingSessionTracker`. |
| **ReadingStatsActivity** | `src/activities/stats/ReadingStatsActivity.h` | Giao diện thống kê 4 tab: Tổng quan (các thẻ chỉ số), Theo sách (danh sách và popup chi tiết), Heatmap (lịch đọc 52 tuần), Thói quen (chuỗi ngày liên tục, thời gian đọc trung bình). |
| **GfxRenderer** | `lib/GfxRenderer/GfxRenderer.h` | Thao tác vẽ trực tiếp lên bộ đệm 1-bit 48KB: đường kẻ, hình tròn, hộp chữ nhật, tô màu dither, bố cục văn bản, và biến đổi tọa độ theo 4 hướng xoay màn hình. |
| **HalStorage** | `lib/hal/HalStorage.h` | Singleton `Storage` cung cấp handle `HalFile` được đồng bộ hóa mutex trên nền SdFat nhằm ngăn chặn xung đột SPI giữa các task. |
| **CrossPointSettings** | `src/CrossPointSettings.h` | Lưu trữ hướng màn hình, kiểu font, lề trang, hành vi nút bấm và theme vào `/.crosspoint/settings.json`. |
| **UITheme** | `src/components/UITheme.h` | Singleton `GUI` cung cấp design token, khoảng cách lề chuẩn, thanh tiêu đề, chân trang, gợi ý phím bấm và khung viền popup. |

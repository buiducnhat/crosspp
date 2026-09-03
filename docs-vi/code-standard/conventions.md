# Quy chuẩn lập trình, Quy ước kỹ thuật & Pattern nhúng

## Giới hạn cốt lõi: Trần 380KB RAM

ESP32-C3 chỉ có ~380KB DRAM khả dụng và **hoàn toàn không có PSRAM**. Hiện tượng phân mảnh bộ nhớ hoặc cấp phát heap không kiểm soát sẽ gây sập thiết bị (panic/crash).

### 1. An toàn bộ nhớ & Giao thức cấp phát Heap
- **`new` không phải nothrow trên ESP32**: Với cờ `-fno-exceptions`, lệnh `new` khi thiếu bộ nhớ sẽ gọi `abort()`. Luôn luôn sử dụng `makeUniqueNoThrow<T>()` từ `lib/Memory/Memory.h` hoặc `new (std::nothrow)`.
- **Cấp phát bộ đệm trên Heap**: Ưu tiên dùng `auto buf = makeUniqueNoThrow<uint8_t[]>(size);` thay vì gọi `malloc`. Cơ chế này tự động giải phóng vùng nhớ khi thoát hàm hoặc gặp lỗi sớm.
- **Ngân sách ngăn xếp (Stack Budget)**: Biến cục bộ trong hàm phải nhỏ hơn **256 bytes**. Stack của task rất hạn chế; các buffer lớn bắt buộc phải nằm trên heap hoặc static pool.
- **Cấp phát trước (`std::vector`)**: Luôn gọi `.reserve(N)` trước khi chạy vòng lặp `push_back()` để tránh việc vector liên tục tái cấp phát và gây phân mảnh DRAM.
- **Xử lý chuỗi (String Policy)**: Nghiêm cấm dùng `std::string` và Arduino `String` trong các luồng render hiển thị. Dùng `std::string_view` cho dữ liệu chỉ đọc (lưu ý: kiểm tra ký tự kết thúc null trước khi truyền vào C API) và dùng mảng `char[]` cố định kèm hàm `snprintf`.

### 2. Kiến trúc & Quy tắc phần cứng
- **SdFat & An toàn đa luồng**: Luôn luôn sử dụng `HalStorage` (macro `Storage`) và `HalFile`. Tuyệt đối không gọi trực tiếp vào `SdFat` hoặc `SdSpiCard`; truy cập đồng thời mà không có `storageMutex` sẽ kích hoạt assert của FreeRTOS và crash hệ thống.
- **`DESTRUCTOR_CLOSES_FILE=1`**: Biến cục bộ kiểu `HalFile` sẽ tự động đóng khi ra khỏi phạm vi khối lệnh (scope). Không gọi `file.close()` thủ công trên biến cục bộ trừ khi cần mở lại hoặc xóa file trên cùng đường dẫn. Các biến thành viên `HalFile` trong class phải được gọi `close()` dứt điểm trong `onExit()`.
- **Căn chỉnh bộ nhớ RISC-V (Alignment)**: ESP32-C3 báo lỗi phần cứng khi đọc dữ liệu nhiều byte không căn chỉnh (unaligned load). Không được ép kiểu con trỏ mảng byte sang con trỏ kiểu số nguyên lớn hơn. Luôn dùng `memcpy` khi deserialize nhị phân.
- **Bộ nhớ Flash vs DRAM (`constexpr` First)**: Các bảng tra cứu lớn và dữ liệu hằng số phải được đánh dấu `constexpr` hoặc `static const` để nằm trên bus Flash (Instruction Bus), tiết kiệm DRAM. Mọi hàm thực thi khi bộ đệm Flash bị ngắt (như hàm ngắt ISR) bắt buộc phải có tiền tố `IRAM_ATTR`.

### 3. Giao diện người dùng & Đa ngôn ngữ (i18n)
- **Hỗ trợ mọi hướng xoay**: Không bao giờ hardcode kích thước màn hình (800 hoặc 480). Luôn lấy thông số động từ `renderer.getScreenWidth()`, `renderer.getScreenHeight()`, hoặc `renderer.getOrientedViewableTRBL()`.
- **UI Theme Macro**: Toàn bộ thao tác vẽ giao diện người dùng phải đi qua macro `GUI` (`UITheme::getInstance()`).
- **Nút bấm Logic**: Luôn dùng enum nút logic `MappedInputManager::Button::*` (ví dụ: `Button::Confirm`, `Button::Back`), không dùng số chân GPIO vật lý.
- **Chuỗi ký tự bản dịch**: Toàn bộ chuỗi hiển thị cho người dùng phải dùng macro `tr(STR_*)` được định nghĩa trong `lib/I18n/translations/english.yaml`. Sau khi sửa đổi file YAML, chạy lệnh cập nhật:
  ```bash
  python3 scripts/gen_i18n.py lib/I18n/translations lib/I18n/
  ```

### 4. Định dạng mã nguồn & Kiểm tra biên dịch
- **Format mã nguồn**: Chạy script wrapper clang-format của dự án:
  ```bash
  ./bin/clang-format-fix -g
  ```
  Không được gọi trực tiếp `clang-format`.
- **Kiểm tra biên dịch**:
  ```bash
  pio run
  ```

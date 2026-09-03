# Mục lục tài liệu kỹ thuật (Documentation Summary)

CrossPP là firmware máy đọc sách mã nguồn mở, gọn nhẹ dành cho các thiết bị e-paper chạy ESP32-C3/S3 (dòng Xteink X3/X4 và các phần cứng tương thích). Dự án nổi bật với engine dàn trang EPUB nhanh, font chữ tùy biến, từ điển offline, quản lý sách không dây và tính năng theo dõi thống kê đọc sách tích hợp sẵn.

## Hướng dẫn nạp ngữ cảnh cho AI Agent (Agent Context Guide)

Trước khi lập kế hoạch hoặc thực hiện sửa đổi mã nguồn, hãy đọc file `docs-vi/SUMMARY.md` (hoặc `docs/SUMMARY.md`) trước tiên. Chỉ tải các tài liệu chi tiết liên quan trực tiếp đến tác vụ hiện tại, và ưu tiên đọc nhóm tài liệu `Quy chuẩn lập trình (Code Standard)` để nắm vững các quy ước của dự án. Nếu nội dung tài liệu mâu thuẫn với mã nguồn thực tế hoặc ý định của người dùng, hãy dùng công cụ hỏi/xác nhận trước khi thực hiện các thay đổi diện rộng.

## Kiến trúc hệ thống (Architecture)

Thiết kế hệ thống, tương tác giữa các thành phần, luồng dữ liệu và tích hợp phần cứng.

| Tệp | Mô tả |
| --- | ----- |
| [components.md](architecture/components.md) | Kiến trúc hệ thống cấp cao, các tầng trừu tượng HAL, pipeline render đồ họa và vòng đời của Activity |
| [kien-truc-he-thong.md](architecture/kien-truc-he-thong.md) | Tổng quan kiến trúc hệ thống, các lớp phần mềm và luồng dữ liệu chính |
| [fork-va-mo-rong.md](architecture/fork-va-mo-rong.md) | Hướng dẫn tạo fork, mở rộng tính năng và tích hợp thiết bị mới |

## Cấu trúc mã nguồn (Codebase)

Sơ đồ thư mục, các điểm vào (entry points) và trách nhiệm của từng module chính.

| Tệp | Mô tả |
| --- | ----- |
| [structure.md](codebase/structure.md) | Bản đồ tổ chức tệp tin, cây thư mục, các file khởi động và bảng phân nhiệm module |
| [cau-truc-thu-muc.md](codebase/cau-truc-thu-muc.md) | Chi tiết cấu trúc thư mục dự án và vị trí các thành phần mã nguồn |

## Quy chuẩn lập trình (Code Standard)

Các quy ước lập trình nhúng, quy tắc an toàn bộ nhớ và quy trình làm việc chuẩn.

| Tệp | Mô tả |
| --- | ----- |
| [conventions.md](code-standard/conventions.md) | Ràng buộc trần 380KB RAM, an toàn bộ nhớ, quy tắc RAII, lệnh định dạng mã nguồn và các bẫy phần cứng ESP32-C3 |
| [quy-uoc-code.md](code-standard/quy-uoc-code.md) | Quy ước đặt tên, phong cách code C++20 và thực hành tốt nhất cho hệ thống nhúng |
| [quy-trinh-phat-trien.md](code-standard/quy-trinh-phat-trien.md) | Quy trình phát triển, kiểm thử, công cụ format và quy chuẩn đóng góp Git |

## Mục tiêu & Yêu cầu sản phẩm (Project PDR)

Tầm nhìn sản phẩm, các tính năng nổi bật và các ranh giới/non-goals kỹ thuật.

| Tệp | Mô tả |
| --- | ----- |
| [product-goals.md](project-pdr/product-goals.md) | Tầm nhìn sản phẩm, các tính năng thống kê đọc sách, quy trình truyền sách không dây và các giới hạn phạm vi |
| [muc-tieu-va-pham-vi.md](project-pdr/muc-tieu-va-pham-vi.md) | Yêu cầu sản phẩm, đối tượng người dùng, phạm vi chức năng và các trường hợp sử dụng |

## Tài liệu tham khảo khác (Other)

Các tài liệu kỹ thuật chuyên sâu và hướng dẫn xử lý sự cố phần cứng hiện có (bản tiếng Anh gốc).

| Tệp | Mô tả |
| --- | ----- |
| [../docs/activity-manager.md](../docs/activity-manager.md) | Mô hình điều hướng và quản lý ngăn xếp màn hình (Activity stack) |
| [../docs/file-formats.md](../docs/file-formats.md) | Cấu trúc định dạng nhị phân của các tệp cache trên thẻ SD (`book.bin`, `section.bin`, `reading_stats.json`) |
| [../docs/dictionary.md](../docs/dictionary.md) | Đặc tả định dạng từ điển StarDict và chi tiết tích hợp |
| [../docs/sd-card-fonts.md](../docs/sd-card-fonts.md) | Kiến trúc font chữ tùy biến `.cpfont` trên thẻ SD và công cụ chuyển đổi |
| [../docs/focus-reading.md](../docs/focus-reading.md) | Chế độ đọc tập trung (bionic reading) |
| [../docs/hyphenation-trie-format.md](../docs/hyphenation-trie-format.md) | Định dạng cây nhị phân (trie) phục vụ thuật toán gạch nối từ tự động |
| [../docs/i18n.md](../docs/i18n.md) | Hệ thống đa ngôn ngữ, sinh bảng chuỗi và hỗ trợ văn bản viết từ phải sang trái (RTL) |
| [../docs/webserver.md](../docs/webserver.md) | Kiến trúc máy chủ Web nhúng và các giao thức truyền tải |
| [../docs/webserver-endpoints.md](../docs/webserver-endpoints.md) | Danh sách API REST phục vụ quản lý tệp và cài đặt qua trình duyệt |
| [../docs/troubleshooting.md](../docs/troubleshooting.md) | Các bước chẩn đoán lỗi và quy trình khôi phục thiết bị |
| [../docs/fix-bricked-xteink.md](../docs/fix-bricked-xteink.md) | Hướng dẫn cứu máy bị brick bằng kẹp nạp bộ nhớ SPI flash |
| [../docs/comparison.md](../docs/comparison.md) | Bảng so sánh tính năng giữa CrossPP và firmware gốc của nhà sản xuất |
| [../docs/translators.md](../docs/translators.md) | Danh sách người đóng góp bản dịch và hướng dẫn tham gia dịch thuật |

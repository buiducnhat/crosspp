# Mục tiêu sản phẩm, Tính năng và Quyết định thiết kế

## Tầm nhìn sản phẩm

CrossPP là firmware máy đọc sách tập trung, không phân tâm, được tối ưu hóa đặc biệt cho thiết bị Xteink X3/X4 và phần cứng màn hình e-paper ESP32 tương thích. Mục tiêu cao nhất là mang lại trải nghiệm đọc sách mượt mà, ổn định cao, tiết kiệm pin tối đa và không có các tính năng rườm rà làm suy giảm độ ổn định bộ nhớ.

---

## Các năng lực cốt lõi của sản phẩm

### 1. Trải nghiệm đọc sách Offline & Cục bộ hoàn toàn
- **Hỗ trợ định dạng phong phú**: Khả năng render nhanh chóng các định dạng `.epub` (chuẩn EPUB 2 và 3), `.txt`, `.bmp`, và truyện tranh `.xtc/.xtch`.
- **Tùy biến kiểu dáng**: Người dùng có thể linh hoạt chuyển đổi giữa định dạng CSS gốc của sách hoặc áp dụng quy tắc font và lề trang đồng nhất.
- **Tích hợp từ điển StarDict**: Tra cứu nghĩa của từ ngay lập tức bằng các tệp từ điển offline đặt trên thẻ SD (`/dict/`).
- **Đồng bộ tiến độ đọc**: Tương thích hoàn toàn với máy chủ đồng bộ KOReader để tiếp tục trang sách đang đọc dở trên các thiết bị khác.

### 2. Thống kê đọc sách tích hợp sẵn (Điểm nhấn của CrossPP)
- **Cục bộ & Bảo mật**: Toàn bộ dữ liệu được lưu trên thẻ SD (`/.crosspoint/reading_stats.json`). Không thu thập dữ liệu người dùng hay yêu cầu kết nối mạng.
- **Tự động đo thời gian đọc**: Chạy ngầm thông minh với bộ đếm timeout 5 phút không hoạt động để tránh ghi nhận sai lệch khi người dùng rời mắt khỏi máy.
- **Bốn góc nhìn phân tích chuyên sâu**:
  1. **Tổng quan (Overview)**: Tổng thời gian đọc, số phiên đọc, số sách đã đọc xong, chuỗi ngày đọc liên tiếp (streak) và trung bình 7 ngày gần nhất.
  2. **Theo sách (By Book)**: Tiến độ đọc, tổng thời gian, số ngày đọc và số phiên trên từng cuốn sách, hỗ trợ mở sách trực tiếp từ popup.
  3. **Biểu đồ nhiệt (Heatmap)**: Lịch đọc sách 52 tuần phong cách GitHub, chia thành các quý ~13 tuần với 4 cấp độ sắc thái hiển thị thói quen trực quan.
  4. **Thói quen đọc (Insights)**: Chuỗi ngày đọc hiện tại, chuỗi kỷ lục, thời gian đọc trung bình 7 ngày & 30 ngày, ngày đọc nhiều nhất, thứ trong tuần đọc nhiều nhất và thời lượng trung bình mỗi phiên.

### 3. Quản lý sách không dây không cần cắm cáp
- **Trình quản lý tệp Web tích hợp**: Kết nối với máy đọc sách qua Wi-Fi hotspot (chế độ AP) hoặc mạng nội bộ (chế độ STA) để tải sách lên, quản lý file và chỉnh cấu hình trực tiếp trên trình duyệt máy tính/điện thoại.
- **Giao thức WebDAV**: Gắn máy đọc sách thành ổ đĩa mạng trên hệ điều hành máy tính.
- **Trình duyệt danh mục sách OPDS**: Tìm kiếm và tải sách trực tiếp từ các thư viện Calibre Content Server, Standard Ebooks và Project Gutenberg.
- **Cập nhật OTA**: Kiểm tra và nạp firmware phiên bản mới trực tiếp từ GitHub Releases.

---

## Những điều sản phẩm chủ động không làm (Non-Goals)
- **Không tích hợp trình duyệt Web đầy đủ**: Kết nối mạng chỉ phục vụ truyền file, tải sách OPDS và đồng bộ tiến độ đọc; không hỗ trợ duyệt web thông thường nhằm giữ độ mượt mà.
- **Không phát âm thanh / Nghe nhạc nền**: Việc giải mã và phát âm thanh sẽ tiêu tốn CPU và dung lượng RAM vốn dĩ dành riêng cho engine dàn trang sách.
- **Không phụ thuộc dịch vụ đám mây bên thứ ba**: Thiết bị hoạt động độc lập và hoàn hảo ngay cả khi không có kết nối Wi-Fi hay tài khoản trực tuyến nào.

[BUG-CRITICAL-01]

Module: Hệ thống lõi (Core System) / Database

Vấn đề: Không lưu dữ liệu sau phiên làm việc.

Mô tả chi tiết: Khi kết thúc chương trình, mọi dữ liệu thao tác của phiên làm việc hiện tại đều bị mất toàn bộ, không được cập nhật hoặc lưu lại vào cơ sở dữ liệu.

[BUG-CRITICAL-02]

Module: Form Đăng nhập / Đăng ký / Quên mật khẩu

Vấn đề: Lỗi bắt phím nhập liệu.

Mô tả chi tiết: Không thể gõ được kí tự "a" vào các ô nhập liệu từ bàn phím (phải dùng copy/paste).

Ghi chú: Duy đã test và chưa tái hiện được lỗi, cần kiểm tra lại bộ gõ hoặc key event của form.

[BUG-CRITICAL-03]

Module: Đồng bộ dữ liệu (Data Sync)

Vấn đề: Trạng thái đồng bộ giữa các module không nhất quán.

Mô tả chi tiết:

Cập nhật dữ liệu bên "Tất cả hóa đơn" nhưng bên "Tất cả đơn hàng" không được update.

Tạo Voucher mới không hiển thị ngay, bắt buộc phải click đồng bộ thời gian thực mới xuất hiện.

[BUG-CRITICAL-04]

Module: Quyền Sales / POS

Vấn đề: Lỗi luồng tạo khách hàng & Tìm kiếm.

Mô tả chi tiết:

Tạo khách hàng mới thành công nhưng không hiển thị trong màn hình POS.

Thanh tìm kiếm trong màn hình POS không hoạt động (Disabled/Bug).

[BUG-CRITICAL-05]

Module: Quyền Thủ kho / Lập phiếu nhập

Vấn đề: Fix cứng số lượng nhập.

Mô tả chi tiết: Không có trường hoặc thanh điều chỉnh số lượng sản phẩm nhập vào kho, hệ thống đang fix cứng số lượng là 10 máy.


[BUG-HIGH-01]

Module: Tìm kiếm sản phẩm (Admin / Tồn kho / Sales)

Vấn đề: Thuật toán tìm kiếm/gợi ý hoạt động sai.

Mô tả chi tiết: Khi gõ một từ khóa (VD: "k"), kết quả trả về bao gồm cả những sản phẩm không chứa từ khóa đó trong tên (2/3 kết quả bị sai lệch).

[BUG-HIGH-02]

Module: Mã giảm giá (Voucher)

Vấn đề: Logic cộng ngày và định dạng dữ liệu lỗi.

Mô tả chi tiết:

Nút gia hạn nhanh (+ ngày) không cộng dồn từ ngày hết hạn hiện tại mà set cứng theo mốc thời gian của nút (VD: 20/10/2026 + 30 ngày -> 30/11/2026).

Mất dấu chấm phân cách hàng nghìn sau khi bấm lưu chỉnh sửa số tiền.

Form "Chỉnh sửa" gặp các lỗi y hệt form "Thêm mới".

[BUG-HIGH-03]

Module: Thống kê lợi nhuận

Vấn đề: Bộ lọc thời gian không hoạt động.

Mô tả chi tiết: Các chỉ số Doanh thu, Lợi nhuận, Tỷ suất không tự động cập nhật lại khi người dùng thay đổi khoảng thời gian lọc dữ liệu.

[BUG-HIGH-04]

Module: Quyền Thủ kho / Lập phiếu nhập

Vấn đề: Lỗi ràng buộc lọc nhà cung cấp.

Mô tả chi tiết: Bộ lọc nhà cung cấp không có tác dụng. Chọn nhà cung cấp A nhưng vẫn chọn và thêm được sản phẩm của nhà cung cấp B vào phiếu nhập.

[BUG-HIGH-05]

Module: Quản lý sản phẩm (Admin)

Vấn đề: Lỗi cập nhật hình ảnh.

Mô tả chi tiết: Tính năng thay đổi/cập nhật ảnh của sản phẩm không hoạt động.

[BUG-HIGH-06]

Module: Hệ thống thời gian thực

Vấn đề: Đồng hồ dừng hoạt động sau đồng bộ.

Mô tả chi tiết: Tính năng "Kiểm soát thời gian thực" có đồng bộ với máy tính nhưng sau đó kim/thời gian không tự động chạy tiếp.
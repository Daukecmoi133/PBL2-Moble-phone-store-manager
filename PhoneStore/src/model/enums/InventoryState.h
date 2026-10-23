#pragma once
// Trạng thái tồn kho của một đơn, lưu trong Order. CHỈ OrderService được đổi trường này.
// Mỗi lượng hàng chỉ được giải phóng/hoàn đúng một lần (dựa vào trạng thái này).

enum class InventoryState {
    NONE,      // Chưa giữ chỗ
    RESERVED,  // Đã giữ chỗ (reservedQuantity += n)
    DEDUCTED,  // Đã xuất kho thật (quantity -= n)
    RELEASED,  // Đã giải phóng giữ chỗ (hủy đơn PENDING)
    RESTOCKED  // Đã hoàn kho (hủy đơn CONFIRMED)
};

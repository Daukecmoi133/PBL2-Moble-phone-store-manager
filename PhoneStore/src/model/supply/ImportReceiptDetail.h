#pragma once
// Chi tiết phiếu nhập. Bản cũ dùng chung OrderDetail (phoneId, phoneName, quantity, unitPrice)
// cho cả đơn hàng lẫn phiếu nhập; giữ nguyên để không đổi định dạng file (docs/MIGRATION.md).

#include "../sales/Order.h"

using ImportReceiptDetail = OrderDetail;

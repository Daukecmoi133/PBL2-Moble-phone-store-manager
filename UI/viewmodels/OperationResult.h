#ifndef OPERATION_RESULT_H
#define OPERATION_RESULT_H

#include <string>

// Kết quả thao tác chuẩn giữa Controller và UI
struct OperationResult {
    bool success;
    int code;
    std::string message;
    std::string details;

    OperationResult() : success(false), code(0), message(""), details("") {}

    OperationResult(bool s, const std::string& msg, int c = 0, const std::string& det = "")
        : success(s), code(c), message(msg), details(det) {}

    bool isSuccess() const { return success; }

    static OperationResult Ok(const std::string& msg = "Thao tác thành công!", const std::string& det = "") {
        return OperationResult(true, msg, 0, det);
    }

    static OperationResult Fail(const std::string& msg, int code = -1, const std::string& det = "") {
        return OperationResult(false, msg, code, det);
    }

    static OperationResult NotConnected(const std::string& featureName) {
        return OperationResult(false, "Chức năng '" + featureName + "' chưa kết nối Service.", -999,
                               "Giao diện đã sẵn sàng tiếp nhận Controller & Service.");
    }
};

#endif // OPERATION_RESULT_H

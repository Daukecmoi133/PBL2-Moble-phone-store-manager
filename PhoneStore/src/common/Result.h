#pragma once
// Result<T>: kết quả của một thao tác, hoặc là giá trị hợp lệ, hoặc là (mã lỗi + thông báo).
// Có thể di chuyển (movable). T không cần mặc định khởi tạo được; giá trị lưu qua unique_ptr.
// Chỉ sao chép được khi T sao chép được (hàm sao chép chỉ được biên dịch khi dùng tới).

#include <memory>
#include <string>
#include <utility>

#include "ErrorCode.h"

template <typename T>
class Result {
public:
    // Tạo kết quả thành công
    static Result ok(T value) {
        Result r;
        r.valuePtr = std::unique_ptr<T>(new T(std::move(value)));
        return r;
    }

    // Tạo kết quả thất bại
    static Result fail(ErrorCode code, std::string message) {
        Result r;
        r.errorCode = code;
        r.errorMessage = std::move(message);
        return r;
    }

    Result(Result&&) noexcept = default;
    Result& operator=(Result&&) noexcept = default;

    Result(const Result& other)
        : valuePtr(other.valuePtr ? new T(*other.valuePtr) : nullptr),
          errorCode(other.errorCode), errorMessage(other.errorMessage) {}

    Result& operator=(const Result& other) {
        if (this != &other) {
            valuePtr.reset(other.valuePtr ? new T(*other.valuePtr) : nullptr);
            errorCode = other.errorCode;
            errorMessage = other.errorMessage;
        }
        return *this;
    }

    bool isOk() const { return errorCode == ErrorCode::Ok; }
    const T& value() const { return *valuePtr; }   // chỉ gọi khi isOk()
    T takeValue() { return std::move(*valuePtr); } // lấy giá trị ra (cho T chỉ di chuyển được); chỉ gọi khi isOk()
    ErrorCode code() const { return errorCode; }
    const std::string& message() const { return errorMessage; }

private:
    Result() : errorCode(ErrorCode::Ok) {}

    std::unique_ptr<T> valuePtr;
    ErrorCode errorCode;
    std::string errorMessage;
};

// Chuyên biệt hóa cho thao tác không trả giá trị
template <>
class Result<void> {
public:
    static Result ok() { return Result(ErrorCode::Ok, ""); }
    static Result fail(ErrorCode code, std::string message) {
        return Result(code, std::move(message));
    }

    bool isOk() const { return errorCode == ErrorCode::Ok; }
    ErrorCode code() const { return errorCode; }
    const std::string& message() const { return errorMessage; }

private:
    Result(ErrorCode c, std::string m) : errorCode(c), errorMessage(std::move(m)) {}

    ErrorCode errorCode;
    std::string errorMessage;
};

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @file FileHelper.h
 * @brief Tiện ích ghi file nguyên tử (Atomic File Write) và an toàn dữ liệu
 * 
 * Đảm bảo:
 * 1. Ghi vào file tạm (.tmp) trước. Chỉ thay thế file đích khi ghi toàn bộ nội dung thành công.
 * 2. Không làm file dữ liệu cũ bị truncate hoặc mất nội dung khi gặp sự cố ghi đĩa.
 * 3. Hỗ trợ mô phỏng lỗi (Failure Injection) để kiểm thử Transaction & Rollback.
 */
class FileHelper {
private:
    inline static std::string s_failureInjectionPath = "";

public:
    // Thiết lập đường dẫn mô phỏng lỗi ghi đĩa (dành cho kiểm thử)
    static void simulateFailureOn(const std::string& pathSubstring) {
        s_failureInjectionPath = pathSubstring;
    }

    // Xóa cờ mô phỏng lỗi
    static void clearSimulatedFailure() {
        s_failureInjectionPath = "";
    }

    // Kiểm tra xem một đường dẫn có đang bị mô phỏng lỗi hay không
    static bool isSimulatedFailure(const std::string& path) {
        if (s_failureInjectionPath.empty()) return false;
        return path.find(s_failureInjectionPath) != std::string::npos;
    }

    /**
     * @brief Đẩy cưỡng bức toàn bộ buffer của hệ điều hành xuống thiết bị lưu trữ vật lý (Durable Flush)
     */
    static void flushFile(const std::string& path) {
#ifdef _WIN32
        HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            FlushFileBuffers(hFile);
            CloseHandle(hFile);
        }
#endif
    }

    /**
     * @brief Ghi dữ liệu nguyên tử thông qua callback
     * @param targetPath Đường dẫn file đích
     * @param writeCallback Lambda nhận std::ofstream& và trả về bool (true nếu ghi dữ liệu hợp lệ)
     * @return true nếu ghi và thay thế file thành công, false nếu thất bại
     */
    template <typename WriteCallback>
    static bool atomicWrite(const std::string& targetPath, WriteCallback writeCallback) {
        // Kiểm tra mô phỏng lỗi kiểm thử
        if (isSimulatedFailure(targetPath)) {
            std::cerr << "[FileHelper Cảnh báo Test] Đã mô phỏng lỗi ghi đĩa có chủ đích trên file: "
                      << targetPath << std::endl;
            return false;
        }

        std::error_code ec;
        std::filesystem::path p(targetPath);
        std::filesystem::path parent = p.parent_path();
        if (!parent.empty() && !std::filesystem::exists(parent, ec)) {
            std::filesystem::create_directories(parent, ec);
            if (ec) {
                std::cerr << "[FileHelper Lỗi] Không thể tạo thư mục cha: " << parent.string()
                          << " (" << ec.message() << ")" << std::endl;
                return false;
            }
        }

        std::string tmpPath = targetPath + ".tmp";

        // 1. Mở file tạm với cờ truncate
        std::ofstream tmpFile(tmpPath, std::ios::out | std::ios::trunc);
        if (!tmpFile.is_open()) {
            std::cerr << "[FileHelper Lỗi] Không thể mở file tạm để ghi: " << tmpPath << std::endl;
            return false;
        }

        // 2. Thực thi ghi dữ liệu
        bool writeSuccess = false;
        try {
            writeSuccess = writeCallback(tmpFile);
        } catch (const std::exception& e) {
            std::cerr << "[FileHelper Ngoại lệ] Lỗi trong callback ghi file: " << e.what() << std::endl;
            writeSuccess = false;
        } catch (...) {
            std::cerr << "[FileHelper Ngoại lệ] Lỗi không xác định trong callback ghi file." << std::endl;
            writeSuccess = false;
        }

        // 3. Kiểm tra tính toàn vẹn của stream trước khi flush/close
        if (!writeSuccess || tmpFile.fail() || !tmpFile.good()) {
            tmpFile.close();
            std::filesystem::remove(tmpPath, ec);
            std::cerr << "[FileHelper Lỗi] Dữ liệu ghi không hợp lệ hoặc stream bị lỗi: " << tmpPath << std::endl;
            return false;
        }

        tmpFile.flush();
        tmpFile.close();

        // 3.5. Đẩy cưỡng bức toàn bộ buffer hệ điều hành xuống thiết bị lưu trữ vật lý (Physical Flush)
        flushFile(tmpPath);

        // 4. Thay thế nguyên tử file chính bằng file tạm
#ifdef _WIN32
        // MoveFileExA với MOVEFILE_REPLACE_EXISTING và MOVEFILE_WRITE_THROUGH đảm bảo ghi flush metadata xuống đĩa
        if (!MoveFileExA(tmpPath.c_str(), targetPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            DWORD winErr = GetLastError();
            std::filesystem::remove(tmpPath, ec);
            std::cerr << "[FileHelper Lỗi] MoveFileExA thất bại với mã lỗi Windows: " << winErr
                      << " cho file " << targetPath << std::endl;
            return false;
        }
#else
        std::filesystem::rename(tmpPath, targetPath, ec);
        if (ec) {
            std::filesystem::remove(tmpPath, ec);
            std::cerr << "[FileHelper Lỗi] std::filesystem::rename thất bại: " << ec.message() << std::endl;
            return false;
        }
#endif

        return true;
    }

    /**
     * @brief Tạo file rỗng với header nếu file chưa tồn tại
     */
    static bool createEmptyFileWithHeader(const std::string& targetPath, const std::string& headerLine) {
        std::error_code ec;
        if (std::filesystem::exists(targetPath, ec)) {
            return true; // File đã tồn tại, không ghi đè
        }
        return atomicWrite(targetPath, [&](std::ofstream& file) {
            file << headerLine << "\n";
            return true;
        });
    }
};

#endif // FILEHELPER_H


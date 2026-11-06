#ifndef PATHHELPER_H
#define PATHHELPER_H

#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @file PathHelper.h
 * @brief Tiện ích giải quyết đường dẫn dữ liệu độc lập với working directory (CWD)
 */
class PathHelper {
public:
    static std::string resolveDataPath(const std::string& relativePath) {
        std::error_code ec;
#ifdef _WIN32
        // 1. Lấy đường dẫn của file thực thi (.exe)
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            std::filesystem::path exePath(buf);
            std::filesystem::path exeDir = exePath.parent_path();

            // Nếu exeDir / "data" tồn tại, ưu tiên giải quyết theo exeDir
            if (std::filesystem::exists(exeDir / "data", ec)) {
                return (exeDir / relativePath).string();
            }

            // Thử exeDir / relativePath
            std::filesystem::path candidate1 = exeDir / relativePath;
            if (std::filesystem::exists(candidate1, ec)) {
                return candidate1.string();
            }

            // Thử exeDir / .. / relativePath (trường hợp chạy từ thư mục con như bin/ hoặc build/ hoặc tests/)
            std::filesystem::path candidate2 = exeDir.parent_path() / relativePath;
            if (std::filesystem::exists(candidate2, ec) || std::filesystem::exists(exeDir.parent_path() / "data", ec)) {
                return candidate2.string();
            }
        }
#endif

        // 2. Kiểm tra trực tiếp từ thư mục làm việc hiện tại (CWD)
        if (std::filesystem::exists(relativePath, ec)) {
            return relativePath;
        }

        return relativePath;
    }
};

#endif // PATHHELPER_H

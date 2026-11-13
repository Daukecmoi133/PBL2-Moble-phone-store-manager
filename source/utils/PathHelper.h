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
private:
    inline static std::string s_customDataDir = "";

public:
    static void setDataRoot(const std::string& dir) { s_customDataDir = dir; }
    static void clearDataRoot() { s_customDataDir = ""; }
    static std::string getDataRoot() {
        if (!s_customDataDir.empty()) return s_customDataDir;
        const char* envData = std::getenv("PBL2_DATA_DIR");
        if (envData && envData[0] != '\0') return std::string(envData);
        const char* envSandbox = std::getenv("PBL2_TEST_SANDBOX");
        if (envSandbox && envSandbox[0] != '\0') return std::string(envSandbox);
        return "";
    }

    static std::string resolveDataPath(const std::string& relativePath) {
        if (relativePath.empty()) return "";
        std::string effectiveRoot = getDataRoot();
        if (!effectiveRoot.empty()) {
            if (relativePath == "data") return effectiveRoot;
            if (relativePath.rfind(effectiveRoot, 0) == 0) {
                return relativePath;
            }
            if (relativePath.rfind("data/", 0) == 0 || relativePath.rfind("data\\", 0) == 0) {
                return effectiveRoot + "/" + relativePath.substr(5);
            }
            return effectiveRoot + "/" + relativePath;
        }

        std::error_code ec;
#ifdef _WIN32
        // Kiểm tra nếu là đường dẫn tuyệt đối hợp lệ đã tồn tại
        std::filesystem::path pAbs(relativePath);
        if (pAbs.is_absolute() && std::filesystem::exists(pAbs, ec)) {
            return relativePath;
        }

        // 1. Lấy đường dẫn của file thực thi (.exe)
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            std::filesystem::path exePath(buf);
            std::filesystem::path exeDir = exePath.parent_path();

            // Nếu exeDir / "data" tồn tại, ưu tiên giải quyết theo exeDir
            if (std::filesystem::exists(exeDir / "data", ec)) {
                if (relativePath.rfind("data/", 0) == 0 || relativePath.rfind("data\\", 0) == 0) {
                    return (exeDir / relativePath).string();
                }
                if (std::filesystem::exists(exeDir / "data" / relativePath, ec)) {
                    return (exeDir / "data" / relativePath).string();
                }
                if (relativePath.rfind("Device/", 0) == 0 || relativePath.rfind("Device\\", 0) == 0 ||
                    relativePath.rfind("people/", 0) == 0 || relativePath.rfind("people\\", 0) == 0 ||
                    relativePath.rfind("PAY/", 0) == 0 || relativePath.rfind("PAY\\", 0) == 0 ||
                    relativePath.rfind("system/", 0) == 0 || relativePath.rfind("system\\", 0) == 0) {
                    return (exeDir / "data" / relativePath).string();
                }
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
                if (std::filesystem::exists(exeDir.parent_path() / "data" / relativePath, ec)) {
                    return (exeDir.parent_path() / "data" / relativePath).string();
                }
                return candidate2.string();
            }
        }
#endif

        // 2. Kiểm tra trực tiếp từ thư mục làm việc hiện tại (CWD)
        if (std::filesystem::exists(relativePath, ec)) {
            return relativePath;
        }
        if (std::filesystem::exists(std::string("data/") + relativePath, ec)) {
            return std::string("data/") + relativePath;
        }

        return relativePath;
    }
};

#endif // PATHHELPER_H

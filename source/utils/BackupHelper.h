#ifndef BACKUPHELPER_H
#define BACKUPHELPER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <openssl/evp.h>

namespace fs = std::filesystem;

/**
 * @class BackupHelper
 * @brief Tiện ích sao lưu, kiểm tra tính toàn vẹn và phục hồi dữ liệu hệ thống (data/)
 *
 * Tạo bản sao lưu timestamped trong thư mục backups/<timestamp>/ kèm manifest CHECKSUMS.sha256.
 * Tính toán checksum bằng OpenSSL SHA-256 chuẩn công nghiệp.
 */
class BackupHelper {
public:
    // Tính mã băm SHA-256 của một tệp tin
    static std::string computeFileSha256(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return "";

        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        if (!mdctx) return "";

        if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }

        char buffer[8192];
        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
            if (EVP_DigestUpdate(mdctx, buffer, file.gcount()) != 1) {
                EVP_MD_CTX_free(mdctx);
                return "";
            }
        }

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int length = 0;
        if (EVP_DigestFinal_ex(mdctx, hash, &length) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
        EVP_MD_CTX_free(mdctx);

        std::ostringstream oss;
        for (unsigned int i = 0; i < length; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return oss.str();
    }

    // Tạo chuỗi timestamp hiện tại dạng YYYYMMDD_HHMMSS
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm tmNow{};
#if defined(_WIN32)
        localtime_s(&tmNow, &nowTime);
#else
        localtime_r(&nowTime, &tmNow);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tmNow);
        return std::string(buf);
    }

    /**
     * @brief Thực hiện sao lưu toàn bộ thư mục nguồn vào thư mục đích kèm mã băm SHA-256
     */
    static bool createBackup(const std::string& sourceDataDir,
                             const std::string& backupsBaseDir,
                             std::string& outBackupPath,
                             std::string& outError) {
        try {
            fs::path srcPath(sourceDataDir);
            if (!fs::exists(srcPath) || !fs::is_directory(srcPath)) {
                outError = "Thu muc nguon khong ton tai: " + sourceDataDir;
                return false;
            }

            std::string timestamp = getCurrentTimestamp();
            fs::path destPath = fs::path(backupsBaseDir) / timestamp;

            // Tạo thư mục backup đích
            fs::create_directories(destPath);

            std::vector<std::pair<std::string, std::string>> checksumEntries;
            uintmax_t totalBytes = 0;
            int totalFiles = 0;

            for (const auto& entry : fs::recursive_directory_iterator(srcPath)) {
                fs::path current = entry.path();
                std::string filename = current.filename().string();

                // Bỏ qua các file tạm thời, lock file, hoặc git
                if (filename == ".app.lock" || filename == ".git" ||
                    filename.find(".tmp") != std::string::npos ||
                    filename.find(".bak") != std::string::npos) {
                    continue;
                }

                fs::path relPath = fs::relative(current, srcPath);
                fs::path targetItem = destPath / relPath;

                if (fs::is_directory(current)) {
                    fs::create_directories(targetItem);
                } else if (fs::is_regular_file(current)) {
                    fs::create_directories(targetItem.parent_path());
                    fs::copy_file(current, targetItem, fs::copy_options::overwrite_existing);

                    std::string sha256 = computeFileSha256(targetItem.string());
                    std::string relNormalized = relPath.generic_string();
                    checksumEntries.emplace_back(sha256, relNormalized);

                    totalBytes += fs::file_size(targetItem);
                    totalFiles++;
                }
            }

            // Ghi file CHECKSUMS.sha256
            fs::path checksumFile = destPath / "CHECKSUMS.sha256";
            std::ofstream cfile(checksumFile);
            if (!cfile.is_open()) {
                outError = "Khong the tao file CHECKSUMS.sha256 tai: " + checksumFile.string();
                return false;
            }
            for (const auto& pair : checksumEntries) {
                cfile << pair.first << "  " << pair.second << "\n";
            }
            cfile.close();

            // Ghi file METADATA.txt
            fs::path metaFile = destPath / "METADATA.txt";
            std::ofstream mfile(metaFile);
            if (mfile.is_open()) {
                mfile << "TIMESTAMP=" << timestamp << "\n";
                mfile << "SOURCE_DIR=" << fs::absolute(srcPath).generic_string() << "\n";
                mfile << "TOTAL_FILES=" << totalFiles << "\n";
                mfile << "TOTAL_BYTES=" << totalBytes << "\n";
                mfile << "VERSION=PBL2-2026\n";
                mfile.close();
            }

            outBackupPath = destPath.generic_string();
            return true;
        } catch (const std::exception& e) {
            outError = "Ngoai le khi sao luu: " + std::string(e.what());
            return false;
        }
    }

    /**
     * @brief Kiểm tra tính toàn vẹn của một bản sao lưu dựa trên CHECKSUMS.sha256
     */
    static bool verifyBackup(const std::string& backupPath, std::string& outError) {
        try {
            fs::path bPath(backupPath);
            fs::path checksumFile = bPath / "CHECKSUMS.sha256";
            if (!fs::exists(checksumFile)) {
                outError = "Thieu file CHECKSUMS.sha256 trong ban sao luu: " + backupPath;
                return false;
            }

            std::ifstream file(checksumFile);
            if (!file.is_open()) {
                outError = "Khong the mo file CHECKSUMS.sha256";
                return false;
            }

            std::string line;
            int verifiedCount = 0;
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                std::size_t spacePos = line.find(' ');
                if (spacePos == std::string::npos) continue;
                std::string expectedHash = line.substr(0, spacePos);
                std::size_t pathStart = line.find_first_not_of(" \t*", spacePos);
                if (pathStart == std::string::npos) continue;
                std::string relPath = line.substr(pathStart);
                while (!relPath.empty() && (relPath.back() == '\r' || relPath.back() == '\n')) {
                    relPath.pop_back();
                }

                fs::path targetFile = bPath / relPath;
                if (!fs::exists(targetFile)) {
                    outError = "File bi thieu trong ban sao luu: " + relPath;
                    return false;
                }

                std::string actualHash = computeFileSha256(targetFile.string());
                if (actualHash != expectedHash) {
                    outError = "Sai ma hash SHA-256 tai file: " + relPath +
                               " (ky vong: " + expectedHash + ", thuc te: " + actualHash + ")";
                    return false;
                }
                verifiedCount++;
            }

            if (verifiedCount == 0) {
                outError = "File CHECKSUMS.sha256 rong hoac khong chua muc hop le nao.";
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            outError = "Ngoai le khi kiem tra sao luu: " + std::string(e.what());
            return false;
        }
    }

    /**
     * @brief Phục hồi dữ liệu từ bản sao lưu sang thư mục đích sau khi đã verify SHA-256
     */
    static bool restoreBackup(const std::string& backupPath,
                             const std::string& targetDataDir,
                             std::string& outError) {
        if (!verifyBackup(backupPath, outError)) {
            outError = "Tu choi phuc hoi do ban sao luu khong vuot qua kiem tra SHA-256: " + outError;
            return false;
        }

        try {
            fs::path bPath(backupPath);
            fs::path targetPath(targetDataDir);
            fs::create_directories(targetPath);

            fs::path checksumFile = bPath / "CHECKSUMS.sha256";
            std::ifstream file(checksumFile);
            std::string line;

            while (std::getline(file, line)) {
                if (line.empty()) continue;
                std::size_t spacePos = line.find(' ');
                if (spacePos == std::string::npos) continue;
                std::size_t pathStart = line.find_first_not_of(" \t*", spacePos);
                if (pathStart == std::string::npos) continue;
                std::string relPath = line.substr(pathStart);
                while (!relPath.empty() && (relPath.back() == '\r' || relPath.back() == '\n')) {
                    relPath.pop_back();
                }

                fs::path srcFile = bPath / relPath;
                fs::path dstFile = targetPath / relPath;

                fs::create_directories(dstFile.parent_path());
                fs::copy_file(srcFile, dstFile, fs::copy_options::overwrite_existing);
            }
            return true;
        } catch (const std::exception& e) {
            outError = "Ngoai le khi phuc hoi: " + std::string(e.what());
            return false;
        }
    }
};

#endif // BACKUPHELPER_H

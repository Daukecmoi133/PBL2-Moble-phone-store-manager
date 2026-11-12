#ifndef TRANSACTION_COORDINATOR_H
#define TRANSACTION_COORDINATOR_H

#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include "FileHelper.h"

/**
 * @file TransactionCoordinator.h
 * @brief Điều phối giao dịch phân tán trên nhiều file (Multi-file Transaction Coordinator)
 * 
 * Cung cấp cơ chế:
 * 1. Transaction Manifest & Commit Marker (.tx_manifest) ghi lại vòng đời giao dịch:
 *    - PREPARING: Đang tạo bản sao lưu snapshot bền vững (.tx_bak).
 *    - ACTIVE: Đã snapshot xong, giao dịch đang diễn ra.
 *    - COMMITTED: Tất cả các bước ghi file đã thành công 100%. Điểm commit không thể đảo ngược.
 * 2. Durable Snapshot: Bản sao lưu được ghi qua .tmp, gọi FlushFileBuffers() trước khi rename.
 * 3. Crash Recovery chống Rollback nhầm: Khi khởi động lại, nếu status là COMMITTED thì KHÔNG rollback,
 *    chỉ dọn dẹp snapshot thừa; nếu status là ACTIVE thì mới rollback an toàn; nếu PREPARING thì bỏ qua.
 */
class FileTransaction {
public:
    struct ParticipantRecord {
        std::string originalPath;
        std::string backupPath;
        bool existedBefore = false;
    };

private:
    std::vector<ParticipantRecord> participants;
    std::vector<std::function<void()>> rollbackCallbacks;
    std::string manifestPath = "";
    bool active = false;
    bool committed = false;

    // Tìm thư mục gốc quản lý dữ liệu (data/ hoặc sandbox_data/ hoặc parent folder)
    static std::string findRootDataDir(const std::string& filePath) {
        std::filesystem::path cur = std::filesystem::absolute(filePath).parent_path();
        while (cur.has_parent_path()) {
            std::string fname = cur.filename().string();
            if (fname == "data" || fname.find("sandbox_data") != std::string::npos) {
                return cur.string();
            }
            if (fname == "Device" || fname == "PAY" || fname == "people" || fname == "system") {
                return cur.parent_path().string();
            }
            cur = cur.parent_path();
        }
        return std::filesystem::path(filePath).parent_path().string();
    }

    void writeManifest(const std::string& status) {
        if (manifestPath.empty()) return;
        FileHelper::atomicWrite(manifestPath, [&](std::ofstream& out) {
            out << "STATUS=" << status << "\n";
            for (const auto& p : participants) {
                out << "PARTICIPANT|" << p.originalPath << "|" << p.backupPath << "|" << (p.existedBefore ? "1" : "0") << "\n";
            }
            return true;
        });
        FileHelper::flushFile(manifestPath);
    }

    void cleanupBackups() {
        std::error_code ec;
        for (const auto& p : participants) {
            if (!p.backupPath.empty()) {
                std::filesystem::remove(p.backupPath, ec);
            }
        }
        if (!manifestPath.empty()) {
            std::filesystem::remove(manifestPath, ec);
        }
    }

public:
    FileTransaction() = default;

    ~FileTransaction() {
        if (active && !committed) {
            rollback();
        }
    }

    // Đăng ký file tham gia giao dịch
    void addParticipant(const std::string& filePath) {
        if (filePath.empty()) return;
        for (const auto& p : participants) {
            if (p.originalPath == filePath) return;
        }
        ParticipantRecord rec;
        rec.originalPath = filePath;
        rec.backupPath = filePath + ".tx_bak";
        participants.push_back(rec);

        if (manifestPath.empty()) {
            manifestPath = findRootDataDir(filePath) + "/.tx_manifest";
        }
    }

    // Đăng ký callback hoàn nguyên RAM (ví dụ: reload lại repository)
    void onRollback(std::function<void()> cb) {
        rollbackCallbacks.push_back(cb);
    }

    // Bắt đầu giao dịch: tạo bản sao lưu an toàn .tx_bak và ghi manifest
    bool begin() {
        if (participants.empty()) {
            active = true;
            committed = false;
            return true;
        }

        if (manifestPath.empty()) {
            manifestPath = findRootDataDir(participants[0].originalPath) + "/.tx_manifest";
        }

        std::error_code ec;
        // Bước 1: Kiểm tra sự tồn tại của file gốc
        for (auto& p : participants) {
            p.existedBefore = std::filesystem::exists(p.originalPath, ec);
        }

        // Bước 2: Ghi manifest với trạng thái PREPARING
        writeManifest("PREPARING");

        // Bước 3: Tạo bản sao lưu an toàn bền vững (.tx_bak)
        for (const auto& p : participants) {
            if (p.existedBefore) {
                std::string tmpBak = p.backupPath + ".tmp";
                std::filesystem::copy_file(p.originalPath, tmpBak,
                                           std::filesystem::copy_options::overwrite_existing, ec);
                if (ec) {
                    std::cerr << "[FileTransaction Lỗi] Không thể copy snapshot tạm: "
                              << tmpBak << " (" << ec.message() << ")" << std::endl;
                    cleanupBackups();
                    return false;
                }
                FileHelper::flushFile(tmpBak);
#ifdef _WIN32
                if (!MoveFileExA(tmpBak.c_str(), p.backupPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
                    std::filesystem::remove(tmpBak, ec);
                    cleanupBackups();
                    return false;
                }
#else
                std::filesystem::rename(tmpBak, p.backupPath, ec);
                if (ec) {
                    std::filesystem::remove(tmpBak, ec);
                    cleanupBackups();
                    return false;
                }
#endif
            }
        }

        // Bước 4: Toàn bộ snapshot đã hoàn tất bền vững trên đĩa -> Cập nhật manifest sang ACTIVE
        writeManifest("ACTIVE");
        active = true;
        committed = false;
        return true;
    }

    // Commit giao dịch: Đánh dấu COMMIT MARKER trước khi dọn dẹp backups
    bool commit() {
        if (!active || committed) return false;

        // Điểm commit quan trọng: Ghi commit marker xuống đĩa trước khi xóa backups
        writeManifest("COMMITTED");

        // Giờ mới an toàn xóa bỏ các file snapshot
        cleanupBackups();
        committed = true;
        active = false;
        return true;
    }

    // Rollback giao dịch: khôi phục nguyên trạng toàn bộ file từ backup và gọi callbacks hoàn nguyên RAM
    void rollback() {
        if (!active || committed) return;

        std::error_code ec;
        for (const auto& p : participants) {
            if (p.existedBefore) {
                if (std::filesystem::exists(p.backupPath, ec)) {
                    // Khôi phục file chính từ backup
                    FileHelper::atomicWrite(p.originalPath, [&](std::ofstream& out) {
                        std::ifstream in(p.backupPath, std::ios::binary);
                        if (!in.is_open()) return false;
                        out << in.rdbuf();
                        return true;
                    });
                }
            } else {
                // Trước giao dịch file này chưa tồn tại, nếu đã được tạo trong giao dịch dở dang thì xóa đi
                if (std::filesystem::exists(p.originalPath, ec)) {
                    std::filesystem::remove(p.originalPath, ec);
                }
            }
        }
        cleanupBackups();

        // Thực thi các callback rollback RAM theo thứ tự ngược lại (LIFO)
        for (auto it = rollbackCallbacks.rbegin(); it != rollbackCallbacks.rend(); ++it) {
            try {
                (*it)();
            } catch (const std::exception& e) {
                std::cerr << "[FileTransaction Ngoại lệ Rollback] " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "[FileTransaction Ngoại lệ Rollback không xác định]" << std::endl;
            }
        }

        active = false;
    }

    bool isActive() const { return active; }
    bool isCommitted() const { return committed; }

    /**
     * @brief Tự động quét và khôi phục sự cố dựa trên Transaction Manifest (.tx_manifest)
     */
    static void recoverPendingTransactions(const std::string& searchDirectory) {
        std::error_code ec;
        if (!std::filesystem::exists(searchDirectory, ec)) return;

        // 1. Quét tìm tất cả các file .tx_manifest
        std::vector<std::string> manifestFiles;
        std::string rootManifest = searchDirectory + "/.tx_manifest";
        if (std::filesystem::exists(rootManifest, ec)) {
            manifestFiles.push_back(rootManifest);
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(searchDirectory, ec)) {
            if (entry.is_regular_file()) {
                std::string fname = entry.path().filename().string();
                if (fname == ".tx_manifest") {
                    std::string p = entry.path().string();
                    bool found = false;
                    for (const auto& m : manifestFiles) {
                        if (m == p) { found = true; break; }
                    }
                    if (!found) manifestFiles.push_back(p);
                }
            }
        }

        // Xử lý từng manifest tìm thấy
        for (const auto& mPath : manifestFiles) {
            std::ifstream in(mPath);
            if (!in.is_open()) continue;

            std::string line;
            std::string status = "UNKNOWN";
            std::vector<ParticipantRecord> records;

            while (std::getline(in, line)) {
                if (line.rfind("STATUS=", 0) == 0) {
                    status = line.substr(7);
                    while (!status.empty() && (status.back() == '\r' || status.back() == '\n' || status.back() == ' ')) {
                        status.pop_back();
                    }
                } else if (line.rfind("PARTICIPANT|", 0) == 0) {
                    std::stringstream ss(line);
                    std::string token;
                    std::vector<std::string> parts;
                    while (std::getline(ss, token, '|')) {
                        parts.push_back(token);
                    }
                    if (parts.size() >= 4) {
                        ParticipantRecord r;
                        r.originalPath = parts[1];
                        r.backupPath = parts[2];
                        r.existedBefore = (parts[3] == "1");
                        records.push_back(r);
                    }
                }
            }
            in.close();

            if (status == "COMMITTED") {
                // TRƯỜNG HỢP 1: Giao dịch ĐÃ COMMIT trước khi crash!
                // TUYỆT ĐỐI KHÔNG ROLLBACK đè lên dữ liệu mới! Chỉ xóa backup dở dang
                std::cout << "[Crash Recovery] Phát hiện giao dịch đã COMMITTED trước sự cố. "
                          << "Bảo toàn dữ liệu đã commit, chỉ dọn dẹp các snapshot thừa." << std::endl;
                for (const auto& r : records) {
                    std::filesystem::remove(r.backupPath, ec);
                }
                std::filesystem::remove(mPath, ec);
            } else if (status == "ACTIVE") {
                // TRƯỜNG HỢP 2: Giao dịch bị crash dở dang chưa commit -> Bắt buộc ROLLBACK
                std::cout << "[Crash Recovery] Phát hiện giao dịch dở dang (ACTIVE). "
                          << "Đang rollback toàn bộ file về trạng thái an toàn trước giao dịch..." << std::endl;
                for (const auto& r : records) {
                    if (r.existedBefore) {
                        if (std::filesystem::exists(r.backupPath, ec)) {
                            FileHelper::atomicWrite(r.originalPath, [&](std::ofstream& out) {
                                std::ifstream bin(r.backupPath, std::ios::binary);
                                if (!bin.is_open()) return false;
                                out << bin.rdbuf();
                                return true;
                            });
                        }
                    } else {
                        if (std::filesystem::exists(r.originalPath, ec)) {
                            std::filesystem::remove(r.originalPath, ec);
                        }
                    }
                    std::filesystem::remove(r.backupPath, ec);
                }
                std::filesystem::remove(mPath, ec);
            } else if (status == "PREPARING") {
                // TRƯỜNG HỢP 3: Crash trong lúc đang tạo snapshot -> Bản backup có thể không hoàn chỉnh,
                // nhưng file gốc CHƯA HỀ bị sửa đổi gì! TUYỆT ĐỐI KHÔNG RESTORE từ snapshot hỏng!
                std::cout << "[Crash Recovery] Giao dịch bị ngắt khi đang tạo snapshot (PREPARING). "
                          << "Dữ liệu gốc hoàn toàn an toàn, đang dọn dẹp snapshot dở dang." << std::endl;
                for (const auto& r : records) {
                    std::filesystem::remove(r.backupPath, ec);
                    std::filesystem::remove(r.backupPath + ".tmp", ec);
                }
                std::filesystem::remove(mPath, ec);
            }
        }

        // 2. Dọn dẹp bất kỳ file .tx_bak mồ côi nào không có manifest (tránh rác đĩa)
        for (const auto& entry : std::filesystem::recursive_directory_iterator(searchDirectory, ec)) {
            if (entry.is_regular_file()) {
                std::string pathStr = entry.path().string();
                if (pathStr.size() > 7 && pathStr.substr(pathStr.size() - 7) == ".tx_bak") {
                    std::filesystem::remove(pathStr, ec);
                }
                if (pathStr.size() > 11 && pathStr.substr(pathStr.size() - 11) == ".tx_bak.tmp") {
                    std::filesystem::remove(pathStr, ec);
                }
            }
        }
    }
};

#endif // TRANSACTION_COORDINATOR_H

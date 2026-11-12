#ifndef WARRANTYREQUESTREPOSITORY_H
#define WARRANTYREQUESTREPOSITORY_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <filesystem>
#include "../models/WarrantyRequest.h"
#include "../core/Vector.h"
#include "../utils/StringHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/FileHelper.h"

/**
 * @class WarrantyRequestRepository
 * @brief Tầng lưu trữ và truy xuất tệp cho yêu cầu bảo hành (Warranty Requests)
 * 
 * Lưu trữ danh sách WarrantyRequest trên RAM bằng Vector<WarrantyRequest>.
 * Đường dẫn mặc định: data/PAY/WarrantyRequests.txt
 * Header: id|orderId|phoneId|phoneName|customerId|createdAt|status|issueDescription|technicianNote|resolution|updatedAt
 */
class WarrantyRequestRepository {
private:
    std::string filePath;
    Vector<WarrantyRequest> requests;
    mutable int errorLinesCount;
    mutable bool hasBackedUpCorrupt;

    bool createEmptyFile() const;

public:
    WarrantyRequestRepository(std::string path = "data/PAY/WarrantyRequests.txt");

    bool loadFromFile();
    bool saveToFile() const;

    const std::string& getFilePath() const { return filePath; }
    int getErrorLinesCount() const { return errorLinesCount; }
    bool isCorruptBackedUp() const { return hasBackedUpCorrupt; }

    const Vector<WarrantyRequest>& getAll() const;
    Vector<WarrantyRequest>& getAll();

    WarrantyRequest* findById(const std::string& id);
    const WarrantyRequest* findById(const std::string& id) const;

    Vector<WarrantyRequest> findByCustomerId(const std::string& customerId) const;
    Vector<WarrantyRequest> findByOrderAndPhone(const std::string& orderId, const std::string& phoneId) const;
    int countOpenByOrderAndPhone(const std::string& orderId, const std::string& phoneId) const;

    bool addRequest(const WarrantyRequest& request);
    bool updateRequest(const WarrantyRequest& updatedRequest);
    bool existsById(const std::string& id) const;
};

// ================= IMPLEMENTATION =================

inline bool WarrantyRequestRepository::createEmptyFile() const {
    return FileHelper::createEmptyFileWithHeader(
        filePath,
        "id|orderId|phoneId|phoneName|customerId|createdAt|status|issueDescription|technicianNote|resolution|updatedAt"
    );
}

inline WarrantyRequestRepository::WarrantyRequestRepository(std::string path)
    : filePath(PathHelper::resolveDataPath(path)),
      errorLinesCount(0),
      hasBackedUpCorrupt(false) {}

inline bool WarrantyRequestRepository::loadFromFile() {
    errorLinesCount = 0;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return createEmptyFile();
    }

    requests.clear();
    std::string line;
    bool isHeader = true;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        line = StringHelper::trim(line);
        if (line.empty()) continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        try {
            WarrantyRequest req = WarrantyRequest::fromFileLine(line);
            if (!req.getId().empty()) {
                requests.push_back(req);
            } else {
                errorLinesCount++;
                std::cerr << "[Canh bao] Dong loi dinh dang tai dong " << lineNum
                          << " trong " << filePath << std::endl;
            }
        } catch (const std::exception& e) {
            errorLinesCount++;
            std::cerr << "[Canh bao] Loi doc dong " << lineNum
                      << " trong " << filePath << ": " << e.what() << std::endl;
        } catch (...) {
            errorLinesCount++;
            std::cerr << "[Canh bao] Loi khong xac dinh tai dong " << lineNum
                      << " trong " << filePath << std::endl;
        }
    }
    file.close();
    return true;
}

inline bool WarrantyRequestRepository::saveToFile() const {
    // AN TOÀN DỮ LIỆU: Nếu có dòng lỗi khi nạp và chưa từng sao lưu trong lần chạy này,
    // sao lưu file gốc thành <file>.corrupt_<YYYYMMDD_HHMMSS> trước lần ghi đè đầu tiên.
    if (errorLinesCount > 0 && !hasBackedUpCorrupt) {
        std::error_code ec;
        if (std::filesystem::exists(filePath, ec)) {
            time_t now = time(nullptr);
            tm* lt = localtime(&now);
            char timeBuf[32];
            strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", lt);
            std::string backupPath = filePath + ".corrupt_" + std::string(timeBuf);

            std::filesystem::copy_file(filePath, backupPath,
                std::filesystem::copy_options::overwrite_existing, ec);
            if (!ec) {
                std::cerr << "[WarrantyRequestRepository] Phat hien " << errorLinesCount
                          << " dong loi khi doc. Da sao luu file goc truoc lan saveToFile dau tien: "
                          << backupPath << std::endl;
            }
        }
        hasBackedUpCorrupt = true; // Chỉ thực hiện một lần mỗi lần chạy
    }

    return FileHelper::atomicWrite(filePath, [&](std::ofstream& file) {
        file << "id|orderId|phoneId|phoneName|customerId|createdAt|status|issueDescription|technicianNote|resolution|updatedAt\n";
        for (int i = 0; i < requests.getSize(); ++i) {
            file << requests[i].toFileString() << "\n";
        }
        return file.good();
    });
}

inline const Vector<WarrantyRequest>& WarrantyRequestRepository::getAll() const {
    return requests;
}

inline Vector<WarrantyRequest>& WarrantyRequestRepository::getAll() {
    return requests;
}

inline WarrantyRequest* WarrantyRequestRepository::findById(const std::string& id) {
    for (int i = 0; i < requests.getSize(); ++i) {
        if (requests[i].getId() == id) return &requests[i];
    }
    return nullptr;
}

inline const WarrantyRequest* WarrantyRequestRepository::findById(const std::string& id) const {
    for (int i = 0; i < requests.getSize(); ++i) {
        if (requests[i].getId() == id) return &requests[i];
    }
    return nullptr;
}

inline Vector<WarrantyRequest> WarrantyRequestRepository::findByCustomerId(const std::string& customerId) const {
    Vector<WarrantyRequest> result;
    for (int i = 0; i < requests.getSize(); ++i) {
        if (requests[i].getCustomerId() == customerId) {
            result.push_back(requests[i]);
        }
    }
    return result;
}

inline Vector<WarrantyRequest> WarrantyRequestRepository::findByOrderAndPhone(const std::string& orderId, const std::string& phoneId) const {
    Vector<WarrantyRequest> result;
    for (int i = 0; i < requests.getSize(); ++i) {
        if (requests[i].getOrderId() == orderId && requests[i].getPhoneId() == phoneId) {
            result.push_back(requests[i]);
        }
    }
    return result;
}

inline int WarrantyRequestRepository::countOpenByOrderAndPhone(const std::string& orderId, const std::string& phoneId) const {
    int count = 0;
    for (int i = 0; i < requests.getSize(); ++i) {
        if (requests[i].getOrderId() == orderId && requests[i].getPhoneId() == phoneId) {
            WarrantyStatus st = requests[i].getStatus();
            if (st == WarrantyStatus::RECEIVED || st == WarrantyStatus::PROCESSING) {
                count++;
            }
        }
    }
    return count;
}

inline bool WarrantyRequestRepository::addRequest(const WarrantyRequest& request) {
    if (existsById(request.getId())) {
        std::cerr << "[Canh bao] ID yeu cau bao hanh da ton tai: " << request.getId() << std::endl;
        return false;
    }
    Vector<WarrantyRequest> backup = requests;
    requests.push_back(request);
    if (!saveToFile()) {
        requests = backup; // Rollback RAM
        return false;
    }
    return true;
}

inline bool WarrantyRequestRepository::updateRequest(const WarrantyRequest& updatedRequest) {
    for (int i = 0; i < requests.getSize(); ++i) {
        if (requests[i].getId() == updatedRequest.getId()) {
            Vector<WarrantyRequest> backup = requests;
            requests[i] = updatedRequest;
            if (!saveToFile()) {
                requests = backup; // Rollback RAM
                return false;
            }
            return true;
        }
    }
    std::cerr << "[Canh bao] Khong tim thay yeu cau bao hanh ID de cap nhat: " << updatedRequest.getId() << std::endl;
    return false;
}

inline bool WarrantyRequestRepository::existsById(const std::string& id) const {
    return findById(id) != nullptr;
}

#endif // WARRANTYREQUESTREPOSITORY_H

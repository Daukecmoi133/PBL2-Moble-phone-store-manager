#ifndef DATAMIGRATOR_H
#define DATAMIGRATOR_H

#include "../core/Vector.h"
#include "../repositories/CustomerRepository.h"
#include "../repositories/DiscountRepository.h"
#include "../repositories/EmployeeRepository.h"
#include "../repositories/InvoiceRepository.h"
#include "../repositories/OrderRepository.h"
#include "../repositories/PhoneRepository.h"
#include "../repositories/ProductDescriptionRepository.h"
#include "../repositories/ProductImageRepository.h"
#include "../repositories/ProductMarketingRepository.h"
#include "../repositories/StockReceiptRepository.h"
#include "../repositories/SupplierRepository.h"
#include "../repositories/WarrantyRepository.h"
#include "../repositories/WarrantyRequestRepository.h"
#include "DateTime.h"
#include "FileHelper.h"
#include "PathHelper.h"
#include "PricingCalculator.h"
#include "ReviewManager.h"
#include "SchemaHelper.h"
#include "StringHelper.h"
#include "TextCodec.h"
#include "TransactionCoordinator.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @file DataMigrator.h
 * @brief Bộ di trú dữ liệu idempotent (v1/v2 -> v3) kèm kiểm đếm bản ghi TRƯỚC/SAU.
 *
 * Bảo đảm:
 *  - Nếu schema_version >= CURRENT_SCHEMA_VERSION: KHÔNG ghi lại bất kỳ file nào (idempotent 100%).
 *  - Nếu cần di trú v1/v2 -> v3:
 *      + Toàn bộ 19 file dữ liệu nằm trong 1 FileTransaction duy nhất.
 *      + Chuẩn hóa header v2/v3 (Customers 12 cột, Employee 13 cột, Orders 14 cột, Invoices 11 cột...).
 *      + Backfill giá vốn snapshot unitCost vào Order/Invoice chưa có từ Products.txt; giữ nguyên trạng thái LEGACY|0 cho hóa đơn cũ trước VAT.
 *      + Bổ sung vat_rate|0.08 vào StoreSettings.txt nếu chưa có và cập nhật schema_version|3.
 *      + Nạp lại toàn bộ file từ đĩa sau khi ghi để đối chiếu số bản ghi TRƯỚC == SAU; rollback nếu sai lệch.
 */
class DataMigrator {
public:
    static constexpr int CURRENT_SCHEMA_VERSION = 3;

    struct FileCountReport {
        std::string relativePath;
        int beforeCount = 0;
        int expectedAfterCount = 0;
        int afterCount = 0;
        bool existedBefore = false;
        bool countMatch = false;
        std::string note;
    };

    struct MigrationResult {
        bool success = false;
        bool migrated = false;
        int versionBefore = 1;
        int versionAfter = 1;
        std::vector<FileCountReport> fileReports;
        std::string errorMessage;
    };

private:
    struct PathRootGuard {
        std::string previousRoot;
        explicit PathRootGuard(const std::string& temporaryRoot)
            : previousRoot(PathHelper::getDataRoot()) {
            PathHelper::setDataRoot(temporaryRoot);
        }
        ~PathRootGuard() {
            if (previousRoot.empty()) {
                PathHelper::clearDataRoot();
            } else {
                PathHelper::setDataRoot(previousRoot);
            }
        }
    };

    static std::string joinPath(const std::string& root, const std::string& rel) {
        if (root.empty()) return rel;
        char last = root.back();
        if (last == '/' || last == '\\') return root + rel;
        return root + "/" + rel;
    }

    static std::vector<std::pair<std::string, std::string>> readRawStoreSettings(const std::string& path) {
        std::vector<std::pair<std::string, std::string>> pairs;
        std::ifstream file(path);
        if (!file.is_open()) return pairs;

        std::string line;
        bool isFirstNonEmpty = true;
        int lineNum = 0;
        while (std::getline(file, line)) {
            ++lineNum;
            if (lineNum == 1) StringHelper::stripUtf8Bom(line);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (StringHelper::trim(line).empty() || line[0] == '#') continue;

            if (isFirstNonEmpty) {
                isFirstNonEmpty = false;
                auto kind = SchemaHelper::inspectFirstLine(
                    line,
                    SchemaHelper::STORE_SETTINGS_HEADER,
                    {},
                    path,
                    [](const std::string& l) {
                        Vector<std::string> p = TextCodec::splitFields(l, '|');
                        if (p.getSize() != 2) return false;
                        std::string k = StringHelper::trim(p[0]);
                        return !k.empty() && k != "key" && k.find(' ') == std::string::npos;
                    });
                if (kind != SchemaHelper::FirstLineKind::DATA_ROW) continue;
            }

            Vector<std::string> parts = TextCodec::splitFields(line, '|');
            if (parts.getSize() >= 2) {
                std::string k = StringHelper::trim(TextCodec::unescape(parts[0]));
                std::string v = TextCodec::unescape(parts[1]);
                if (k.empty() || k == "key") continue;
                bool updated = false;
                for (auto& kv : pairs) {
                    if (kv.first == k) {
                        kv.second = v;
                        updated = true;
                        break;
                    }
                }
                if (!updated) pairs.emplace_back(k, v);
            }
        }
        return pairs;
    }

    static bool writeRawStoreSettings(
        const std::string& path,
        const std::vector<std::pair<std::string, std::string>>& pairs) {
        return FileHelper::atomicWrite(path, [&](std::ofstream& out) {
            out << SchemaHelper::STORE_SETTINGS_HEADER << "\n";
            for (const auto& kv : pairs) {
                out << TextCodec::escapeField(kv.first) << "|"
                    << TextCodec::escapeField(kv.second) << "\n";
            }
            return out.good();
        });
    }

    static std::vector<std::pair<std::string, std::string>> readSupplierProducts(const std::string& path) {
        std::vector<std::pair<std::string, std::string>> pairs;
        std::ifstream mf(path);
        if (!mf.is_open()) return pairs;

        std::string line;
        bool isFirstNonEmpty = true;
        int lineNum = 0;
        while (std::getline(mf, line)) {
            ++lineNum;
            if (lineNum == 1) StringHelper::stripUtf8Bom(line);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (StringHelper::trim(line).empty() || line[0] == '#') continue;

            if (isFirstNonEmpty) {
                isFirstNonEmpty = false;
                auto kind = SchemaHelper::inspectFirstLine(
                    line,
                    SchemaHelper::SUPPLIER_PRODUCTS_HEADER,
                    {},
                    path,
                    [](const std::string& l) {
                        return SchemaHelper::looksLikePrefixedRecord(l, "SUP");
                    });
                if (kind != SchemaHelper::FirstLineKind::DATA_ROW) continue;
            }

            Vector<std::string> parts = TextCodec::splitFields(line, '|');
            if (parts.getSize() >= 2) {
                std::string sup = StringHelper::trim(TextCodec::unescape(parts[0]));
                std::string phn = StringHelper::trim(TextCodec::unescape(parts[1]));
                if (!sup.empty() && !phn.empty()) {
                    pairs.emplace_back(sup, phn);
                }
            }
        }
        return pairs;
    }

    static bool writeSupplierProducts(
        const std::string& path,
        const std::vector<std::pair<std::string, std::string>>& pairs) {
        return FileHelper::atomicWrite(path, [&](std::ofstream& out) {
            out << SchemaHelper::SUPPLIER_PRODUCTS_HEADER << "\n";
            for (const auto& p : pairs) {
                out << TextCodec::escapeField(p.first) << "|"
                    << TextCodec::escapeField(p.second) << "\n";
            }
            return out.good();
        });
    }

    static std::vector<std::string> readRawLinesClean(const std::string& path) {
        std::vector<std::string> lines;
        std::ifstream in(path);
        if (!in.is_open()) return lines;
        std::string line;
        int lineNum = 0;
        while (std::getline(in, line)) {
            ++lineNum;
            if (lineNum == 1) StringHelper::stripUtf8Bom(line);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (StringHelper::trim(line).empty()) continue;
            lines.push_back(line);
        }
        return lines;
    }

    static bool writeRawLinesClean(const std::string& path, const std::vector<std::string>& lines) {
        return FileHelper::atomicWrite(path, [&](std::ofstream& out) {
            for (const auto& l : lines) {
                out << l << "\n";
            }
            return out.good();
        });
    }

    static bool readRealTimeValue(const std::string& path, DateTime& outDt) {
        std::ifstream in(path);
        if (!in.is_open()) return false;
        std::string line;
        if (std::getline(in, line)) {
            StringHelper::stripUtf8Bom(line);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            return DateTime::tryParse(StringHelper::trim(line), outDt);
        }
        return false;
    }

    static bool writeRealTimeValue(const std::string& path, const DateTime& dt) {
        return FileHelper::atomicWrite(path, [&](std::ofstream& out) {
            out << dt.toString() << "\n";
            return out.good();
        });
    }

    static int countDetailedSpecPhones(const PhoneRepository& repo) {
        int count = 0;
        const Vector<Phone>& all = repo.getAll();
        for (int i = 0; i < all.getSize(); ++i) {
            if (!all[i].getDetailedSpecifications().empty()) {
                ++count;
            }
        }
        return count;
    }

    static std::vector<FileCountReport> collectCurrentCounts(const std::string& dataRoot) {
        namespace fs = std::filesystem;
        std::error_code ec;
        std::vector<FileCountReport> reports;

        auto add = [&](const std::string& rel, int count, const std::string& note = "") {
            FileCountReport r;
            r.relativePath = rel;
            r.existedBefore = fs::exists(joinPath(dataRoot, rel), ec);
            r.beforeCount = count;
            r.expectedAfterCount = count;
            r.afterCount = count;
            r.countMatch = true;
            r.note = note;
            reports.push_back(r);
        };

        PhoneRepository phoneRepo;
        if (fs::exists(joinPath(dataRoot, "Device/Products.txt"), ec)) phoneRepo.loadFromFile();
        add("Device/Products.txt", phoneRepo.getAll().getSize());
        add("Device/Thongsokythuat.txt", countDetailedSpecPhones(phoneRepo), "block thong so ghep voi PHN");

        ProductDescriptionRepository descRepo;
        if (fs::exists(joinPath(dataRoot, "Device/ProductDescriptions.txt"), ec)) descRepo.loadFromFile();
        add("Device/ProductDescriptions.txt", static_cast<int>(descRepo.getAll().size()));

        ProductImageRepository imgRepo;
        if (fs::exists(joinPath(dataRoot, "Device/ProductImage.txt"), ec)) imgRepo.loadFromFile();
        add("Device/ProductImage.txt", static_cast<int>(imgRepo.getAll().size()));

        ProductMarketingRepository mktRepo;
        if (fs::exists(joinPath(dataRoot, "Device/ProductMarketing.txt"), ec)) mktRepo.loadFromFile();
        add("Device/ProductMarketing.txt", static_cast<int>(mktRepo.getAll().size()));

        int revCount = fs::exists(joinPath(dataRoot, "Device/ProductReviews.txt"), ec)
                           ? static_cast<int>(ReviewManager::loadAllReviews().size())
                           : 0;
        add("Device/ProductReviews.txt", revCount);

        StockReceiptRepository stockRepo;
        if (fs::exists(joinPath(dataRoot, "Device/StockReceipts.txt"), ec)) stockRepo.loadFromFile();
        add("Device/StockReceipts.txt", stockRepo.getAll().getSize());

        auto supProds = readSupplierProducts(joinPath(dataRoot, "Device/SupplierProducts.txt"));
        add("Device/SupplierProducts.txt", static_cast<int>(supProds.size()));

        DiscountRepository discRepo;
        if (fs::exists(joinPath(dataRoot, "PAY/Discount.txt"), ec)) discRepo.loadFromFile();
        add("PAY/Discount.txt", discRepo.getAll().getSize());

        InvoiceRepository invRepo;
        if (fs::exists(joinPath(dataRoot, "PAY/Invoices.txt"), ec)) invRepo.loadFromFile();
        add("PAY/Invoices.txt", invRepo.getAll().getSize());

        OrderRepository ordRepo;
        if (fs::exists(joinPath(dataRoot, "PAY/Order/Orders.txt"), ec)) ordRepo.loadFromFile();
        add("PAY/Order/Orders.txt", ordRepo.getAllOrders().getSize());

        WarrantyRequestRepository wreqRepo;
        if (fs::exists(joinPath(dataRoot, "PAY/WarrantyRequests.txt"), ec)) wreqRepo.loadFromFile();
        add("PAY/WarrantyRequests.txt", wreqRepo.getAll().getSize());

        CustomerRepository custRepo;
        if (fs::exists(joinPath(dataRoot, "people/Customers.txt"), ec)) custRepo.loadFromFile();
        add("people/Customers.txt", custRepo.getAll().getSize());

        EmployeeRepository empRepo;
        if (fs::exists(joinPath(dataRoot, "people/Employee.txt"), ec)) empRepo.loadFromFile();
        add("people/Employee.txt", empRepo.getAll().getSize());

        SupplierRepository supRepo;
        if (fs::exists(joinPath(dataRoot, "people/Suppliers.txt"), ec)) supRepo.loadFromFile();
        add("people/Suppliers.txt", supRepo.getAll().getSize());

        auto rawSettings = readRawStoreSettings(joinPath(dataRoot, "system/StoreSettings.txt"));
        add("system/StoreSettings.txt", static_cast<int>(rawSettings.size()));

        auto idCounters = readRawLinesClean(joinPath(dataRoot, "system/id_counters.txt"));
        add("system/id_counters.txt", static_cast<int>(idCounters.size()));

        DateTime rt(0, 0, 0, 0, 0, 0);
        int rtCount = (fs::exists(joinPath(dataRoot, "realTime.txt"), ec) &&
                       readRealTimeValue(joinPath(dataRoot, "realTime.txt"), rt))
                          ? 1
                          : 0;
        add("realTime.txt", rtCount);

        WarrantyRepository warRepo;
        if (fs::exists(joinPath(dataRoot, "warranty/Warranties.txt"), ec)) warRepo.loadFromFile();
        add("warranty/Warranties.txt", warRepo.getAll().getSize());

        return reports;
    }

public:
    static int readSchemaVersion(const std::string& dataRoot) {
        const std::string settingsPath = joinPath(dataRoot, "system/StoreSettings.txt");
        auto pairs = readRawStoreSettings(settingsPath);
        for (const auto& kv : pairs) {
            if (kv.first == "schema_version") {
                try {
                    return std::stoi(StringHelper::trim(kv.second));
                } catch (...) {
                    return 1;
                }
            }
        }
        return 1;
    }

    static MigrationResult migrateToLatest(const std::string& dataRoot) {
        MigrationResult res;
        namespace fs = std::filesystem;
        std::error_code ec;

        if (dataRoot.empty() || !fs::exists(dataRoot, ec)) {
            res.errorMessage = "Thu muc du lieu khong ton tai: " + dataRoot;
            return res;
        }

        PathRootGuard rootGuard(dataRoot);
        res.versionBefore = readSchemaVersion(dataRoot);

        // Nếu đã đạt phiên bản mới nhất: KHÔNG ghi lại file nào (idempotent 100%)
        if (res.versionBefore >= CURRENT_SCHEMA_VERSION) {
            res.fileReports = collectCurrentCounts(dataRoot);
            res.versionAfter = res.versionBefore;
            res.migrated = false;
            res.success = true;
            return res;
        }

        // 1. Nạp toàn bộ dữ liệu v1 vào RAM & ghi nhận số bản ghi TRƯỚC
        PhoneRepository phoneRepo;
        const bool hasProducts = fs::exists(joinPath(dataRoot, "Device/Products.txt"), ec);
        if (hasProducts) phoneRepo.loadFromFile();
        const bool hasSpecs = fs::exists(joinPath(dataRoot, "Device/Thongsokythuat.txt"), ec);

        ProductDescriptionRepository descRepo;
        const bool hasDesc = fs::exists(joinPath(dataRoot, "Device/ProductDescriptions.txt"), ec);
        if (hasDesc) descRepo.loadFromFile();

        ProductImageRepository imgRepo;
        const bool hasImg = fs::exists(joinPath(dataRoot, "Device/ProductImage.txt"), ec);
        if (hasImg) imgRepo.loadFromFile();

        ProductMarketingRepository mktRepo;
        const bool hasMkt = fs::exists(joinPath(dataRoot, "Device/ProductMarketing.txt"), ec);
        if (hasMkt) mktRepo.loadFromFile();

        const bool hasReviews = fs::exists(joinPath(dataRoot, "Device/ProductReviews.txt"), ec);
        std::vector<ProductReviewViewModel> reviews =
            hasReviews ? ReviewManager::loadAllReviews() : std::vector<ProductReviewViewModel>{};

        StockReceiptRepository stockRepo;
        const bool hasStock = fs::exists(joinPath(dataRoot, "Device/StockReceipts.txt"), ec);
        if (hasStock) stockRepo.loadFromFile();

        const std::string supProdPath = joinPath(dataRoot, "Device/SupplierProducts.txt");
        const bool hasSupProd = fs::exists(supProdPath, ec);
        auto supProdPairs = hasSupProd ? readSupplierProducts(supProdPath)
                                       : std::vector<std::pair<std::string, std::string>>{};

        DiscountRepository discRepo;
        const bool hasDisc = fs::exists(joinPath(dataRoot, "PAY/Discount.txt"), ec);
        if (hasDisc) discRepo.loadFromFile();

        InvoiceRepository invRepo;
        const bool hasInv = fs::exists(joinPath(dataRoot, "PAY/Invoices.txt"), ec);
        if (hasInv) invRepo.loadFromFile();

        OrderRepository ordRepo;
        const bool hasOrd = fs::exists(joinPath(dataRoot, "PAY/Order/Orders.txt"), ec);
        if (hasOrd) ordRepo.loadFromFile();

        WarrantyRequestRepository wreqRepo;
        const bool hasWreq = fs::exists(joinPath(dataRoot, "PAY/WarrantyRequests.txt"), ec);
        if (hasWreq) wreqRepo.loadFromFile();

        CustomerRepository custRepo;
        const bool hasCust = fs::exists(joinPath(dataRoot, "people/Customers.txt"), ec);
        if (hasCust) custRepo.loadFromFile();

        EmployeeRepository empRepo;
        const bool hasEmp = fs::exists(joinPath(dataRoot, "people/Employee.txt"), ec);
        if (hasEmp) empRepo.loadFromFile();

        SupplierRepository supRepo;
        const bool hasSup = fs::exists(joinPath(dataRoot, "people/Suppliers.txt"), ec);
        if (hasSup) supRepo.loadFromFile();

        const std::string settingsPath = joinPath(dataRoot, "system/StoreSettings.txt");
        auto rawSettingsBefore = readRawStoreSettings(settingsPath);
        bool hadSchemaKey = false;
        bool hadVatRateKey = false;
        auto updatedSettings = rawSettingsBefore;
        for (auto& kv : updatedSettings) {
            if (kv.first == "schema_version") {
                kv.second = std::to_string(CURRENT_SCHEMA_VERSION);
                hadSchemaKey = true;
            } else if (kv.first == "vat_rate") {
                hadVatRateKey = true;
            }
        }
        if (!hadSchemaKey) {
            updatedSettings.emplace_back("schema_version", std::to_string(CURRENT_SCHEMA_VERSION));
        }
        if (!hadVatRateKey) {
            updatedSettings.emplace_back("vat_rate", PricingCalculator::formatVatRateDisk(PricingCalculator::DEFAULT_VAT_RATE));
        }

        // Backfill giá vốn snapshot unitCost vào các dòng Order/Invoice cũ (nếu chưa có)
        std::unordered_map<std::string, double> importPriceByPhone;
        const Vector<Phone>& allPhones = phoneRepo.getAll();
        for (int i = 0; i < allPhones.getSize(); ++i) {
            importPriceByPhone[allPhones[i].getID()] = allPhones[i].getImportPrice();
        }
        Vector<Order>& loadedOrders = ordRepo.getAllOrders();
        for (int oi = 0; oi < loadedOrders.getSize(); ++oi) {
            Vector<OrderDetail>& dets = loadedOrders[oi].getDetails();
            for (int di = 0; di < dets.getSize(); ++di) {
                if (!dets[di].hasUnitCost()) {
                    auto it = importPriceByPhone.find(dets[di].getPhoneId());
                    if (it != importPriceByPhone.end()) {
                        dets[di].setUnitCost(it->second);
                    }
                }
            }
        }
        Vector<Invoice>& loadedInvoices = invRepo.getAll();
        for (int ii = 0; ii < loadedInvoices.getSize(); ++ii) {
            Vector<OrderDetail>& dets = loadedInvoices[ii].getDetails();
            for (int di = 0; di < dets.getSize(); ++di) {
                if (!dets[di].hasUnitCost()) {
                    auto it = importPriceByPhone.find(dets[di].getPhoneId());
                    if (it != importPriceByPhone.end()) {
                        dets[di].setUnitCost(it->second);
                    }
                }
            }
        }

        const std::string idCounterPath = joinPath(dataRoot, "system/id_counters.txt");
        const bool hasCounters = fs::exists(idCounterPath, ec);
        auto idCounters = hasCounters ? readRawLinesClean(idCounterPath) : std::vector<std::string>{};

        const std::string rtPath = joinPath(dataRoot, "realTime.txt");
        const bool hasRt = fs::exists(rtPath, ec);
        DateTime rtDt(2026, 10, 17, 12, 0, 0);
        const bool hasValidRt = hasRt && readRealTimeValue(rtPath, rtDt);

        WarrantyRepository warRepo;
        const bool hasWar = fs::exists(joinPath(dataRoot, "warranty/Warranties.txt"), ec);
        if (hasWar) warRepo.loadFromFile();

        std::vector<FileCountReport> beforeReports = collectCurrentCounts(dataRoot);

        // 2. Mở 1 FileTransaction duy nhất bao trùm toàn bộ file tham gia di trú
        FileTransaction tx;
        if (hasProducts) tx.addParticipant(phoneRepo.getFilePath());
        if (hasSpecs) tx.addParticipant(phoneRepo.getSpecificationsFilePath());
        if (hasDesc) tx.addParticipant(descRepo.getResolvedFilePath());
        if (hasImg) tx.addParticipant(imgRepo.getResolvedFilePath());
        if (hasMkt) tx.addParticipant(mktRepo.getResolvedFilePath());
        if (hasReviews) tx.addParticipant(ReviewManager::getFilePath());
        if (hasStock) tx.addParticipant(stockRepo.getFilePath());
        if (hasSupProd) tx.addParticipant(supProdPath);
        if (hasDisc) tx.addParticipant(discRepo.getFilePath());
        if (hasInv) tx.addParticipant(invRepo.getFilePath());
        if (hasOrd) tx.addParticipant(ordRepo.getFilePath());
        if (hasWreq) tx.addParticipant(wreqRepo.getFilePath());
        if (hasCust) tx.addParticipant(custRepo.getFilePath());
        if (hasEmp) tx.addParticipant(empRepo.getFilePath());
        if (hasSup) tx.addParticipant(supRepo.getFilePath());
        tx.addParticipant(settingsPath);
        if (hasCounters) tx.addParticipant(idCounterPath);
        if (hasValidRt) tx.addParticipant(rtPath);
        if (hasWar) tx.addParticipant(warRepo.getFilePath());

        if (!tx.begin()) {
            res.errorMessage = "Khong the khoi tao FileTransaction cho qua trinh di tru du lieu!";
            return res;
        }

        // 3. Ghi toàn bộ theo chuẩn v2
        bool allSaved = true;
        auto checkSave = [&](bool ok, const std::string& label) {
            if (!ok) {
                allSaved = false;
                if (res.errorMessage.empty()) {
                    res.errorMessage = "Loi khi ghi file v2: " + label;
                }
            }
        };

        if (hasProducts) checkSave(phoneRepo.saveToFile(), "Device/Products.txt");
        if (hasSpecs) checkSave(phoneRepo.saveDetailedSpecifications(), "Device/Thongsokythuat.txt");
        if (hasDesc) checkSave(descRepo.saveToFile(), "Device/ProductDescriptions.txt");
        if (hasImg) checkSave(imgRepo.saveToFile(), "Device/ProductImage.txt");
        if (hasMkt) checkSave(mktRepo.saveToFile(), "Device/ProductMarketing.txt");
        if (hasReviews) checkSave(ReviewManager::saveAllReviews(reviews), "Device/ProductReviews.txt");
        if (hasStock) checkSave(stockRepo.saveToFile(), "Device/StockReceipts.txt");
        if (hasSupProd) checkSave(writeSupplierProducts(supProdPath, supProdPairs), "Device/SupplierProducts.txt");
        if (hasDisc) checkSave(discRepo.saveToFile(), "PAY/Discount.txt");
        if (hasInv) checkSave(invRepo.saveToFile(), "PAY/Invoices.txt");
        if (hasOrd) checkSave(ordRepo.saveToFile(), "PAY/Order/Orders.txt");
        if (hasWreq) checkSave(wreqRepo.saveToFile(), "PAY/WarrantyRequests.txt");
        if (hasCust) checkSave(custRepo.saveToFile(), "people/Customers.txt");
        if (hasEmp) checkSave(empRepo.saveToFile(), "people/Employee.txt");
        if (hasSup) checkSave(supRepo.saveToFile(), "people/Suppliers.txt");
        checkSave(writeRawStoreSettings(settingsPath, updatedSettings), "system/StoreSettings.txt");
        if (hasCounters) checkSave(writeRawLinesClean(idCounterPath, idCounters), "system/id_counters.txt");
        if (hasValidRt) checkSave(writeRealTimeValue(rtPath, rtDt), "realTime.txt");
        if (hasWar) checkSave(warRepo.saveToFile(), "warranty/Warranties.txt");

        if (!allSaved) {
            tx.rollback();
            return res;
        }

        // 4. Đọc lại từ đĩa và đối chiếu số bản ghi TRƯỚC == SAU trên TỪNG FILE
        std::vector<FileCountReport> afterReports = collectCurrentCounts(dataRoot);
        bool countsOk = (beforeReports.size() == afterReports.size());
        std::vector<FileCountReport> mergedReports;
        for (size_t i = 0; i < beforeReports.size() && i < afterReports.size(); ++i) {
            FileCountReport rep = beforeReports[i];
            rep.afterCount = afterReports[i].afterCount;
            if (rep.relativePath == "system/StoreSettings.txt") {
                int addedSettings = (!hadSchemaKey ? 1 : 0) + (!hadVatRateKey ? 1 : 0);
                rep.expectedAfterCount = rep.beforeCount + addedSettings;
                if (addedSettings > 0) {
                    rep.note = "Cap nhat schema_version|3 / vat_rate";
                }
            } else {
                rep.expectedAfterCount = rep.beforeCount;
            }
            rep.countMatch = (rep.afterCount == rep.expectedAfterCount);
            if (!rep.countMatch) {
                countsOk = false;
                if (res.errorMessage.empty()) {
                    res.errorMessage = "Sai lech so ban ghi tai " + rep.relativePath +
                                       ": truoc=" + std::to_string(rep.beforeCount) +
                                       ", ky vong=" + std::to_string(rep.expectedAfterCount) +
                                       ", sau=" + std::to_string(rep.afterCount);
                }
            }
            mergedReports.push_back(rep);
        }

        if (!countsOk) {
            tx.rollback();
            res.fileReports = mergedReports;
            return res;
        }

        if (!tx.commit()) {
            tx.rollback();
            res.errorMessage = "Khong the commit FileTransaction sau di tru du lieu!";
            return res;
        }

        res.fileReports = mergedReports;
        res.versionAfter = readSchemaVersion(dataRoot);
        res.migrated = true;
        res.success = true;
        return res;
    }

    static void printReport(const MigrationResult& res, std::ostream& out = std::cout) {
        out << "[DataMigrator] schema_version: v" << res.versionBefore
            << " -> v" << res.versionAfter
            << " | migrated=" << (res.migrated ? "YES" : ("NO (already v" + std::to_string(CURRENT_SCHEMA_VERSION) + ", idempotent skip)"))
            << " | success=" << (res.success ? "YES" : "NO") << "\n";
        if (!res.errorMessage.empty()) {
            out << "[DataMigrator Error] " << res.errorMessage << "\n";
        }
        out << "File                               | Existed | Before | Expected | After | Match\n";
        out << "-----------------------------------+---------+--------+----------+-------+------\n";
        for (const auto& r : res.fileReports) {
            out << std::left << std::setw(35) << r.relativePath << "| "
                << std::setw(7) << (r.existedBefore ? "YES" : "NO") << " | "
                << std::right << std::setw(6) << r.beforeCount << " | "
                << std::setw(8) << r.expectedAfterCount << " | "
                << std::setw(5) << r.afterCount << " | "
                << (r.countMatch ? "OK" : "FAIL")
                << (r.note.empty() ? "" : (" (" + r.note + ")")) << "\n";
        }
        out << std::left;
    }
};

#endif // DATAMIGRATOR_H

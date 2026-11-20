#ifndef STORE_SETTINGS_REPOSITORY_H
#define STORE_SETTINGS_REPOSITORY_H

#include "../utils/FileHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/PricingCalculator.h"
#include "../utils/StringHelper.h"
#include "../utils/TextCodec.h"
#include "../utils/SchemaHelper.h"
#include <fstream>
#include <map>
#include <string>

class StoreSettingsRepository {
private:
    std::string filePath;
    std::map<std::string, std::string> settings;

    void initDefaults() {
        if (settings.find("store_shipping_policy") == settings.end()) {
            settings["store_shipping_policy"] = "Miễn phí vận chuyển hỏa tốc toàn quốc & Cho phép đồng kiểm trước khi nhận hàng";
        }
        if (settings.find("store_return_policy") == settings.end()) {
            settings["store_return_policy"] = "1 đổi 1 trong vòng 30 ngày nếu có lỗi từ nhà sản xuất";
        }
        if (settings.find("store_warranty_policy") == settings.end()) {
            settings["store_warranty_policy"] = "Bảo hành chính hãng 12 tháng tại các trung tâm bảo hành ủy quyền trên toàn quốc";
        }
        if (settings.find("vat_rate") == settings.end()) {
            settings["vat_rate"] = PricingCalculator::formatVatRateDisk(PricingCalculator::DEFAULT_VAT_RATE);
        }
    }

public:
    StoreSettingsRepository(std::string path = "data/system/StoreSettings.txt")
        : filePath(path) {
        initDefaults();
    }

    const std::string& getFilePath() const { return filePath; }
    std::string getResolvedFilePath() const { return PathHelper::resolveDataPath(filePath); }
    const std::map<std::string, std::string>& getAll() const { return settings; }
    bool hasKey(const std::string& key) const { return settings.find(key) != settings.end(); }

    bool loadFromFile() {
        std::string resolved = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolved);
        if (!file.is_open()) {
            initDefaults();
            return false;
        }

        settings.clear();
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
                    resolved,
                    [](const std::string& l) {
                        Vector<std::string> p = TextCodec::splitFields(l, '|');
                        if (p.getSize() != 2) return false;
                        std::string k = StringHelper::trim(p[0]);
                        return !k.empty() && k != "key" && k.find(' ') == std::string::npos;
                    });
                if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
                    continue;
                }
            }

            Vector<std::string> parts = TextCodec::splitFields(line, '|');
            if (parts.getSize() >= 2) {
                std::string k = StringHelper::trim(TextCodec::unescape(parts[0]));
                std::string v = TextCodec::unescape(parts[1]);
                if (k != "key" && !k.empty()) {
                    settings[k] = v;
                }
            }
        }
        initDefaults();
        return true;
    }

    bool saveToFile() const {
        std::string resolved = PathHelper::resolveDataPath(filePath);
        return FileHelper::atomicWrite(resolved, [&](std::ofstream& out) {
            out << SchemaHelper::STORE_SETTINGS_HEADER << "\n";
            for (const auto& kv : settings) {
                out << TextCodec::escapeField(kv.first) << "|"
                    << TextCodec::escapeField(kv.second) << "\n";
            }
            return out.good();
        });
    }

    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it = settings.find(key);
        if (it != settings.end()) return it->second;
        return defaultValue;
    }

    void set(const std::string& key, const std::string& value) {
        settings[key] = value;
    }

    double getVatRate() const {
        auto it = settings.find("vat_rate");
        if (it != settings.end()) {
            double parsed = PricingCalculator::DEFAULT_VAT_RATE;
            if (PricingCalculator::tryParseVatRate(it->second, parsed)) {
                return parsed;
            }
        }
        return PricingCalculator::DEFAULT_VAT_RATE;
    }

    bool setVatRate(double rate) {
        if (std::isnan(rate) || std::isinf(rate) || rate < 0.0 || rate > 1.0) {
            return false;
        }
        settings["vat_rate"] = PricingCalculator::formatVatRateDisk(rate);
        return true;
    }
};

#endif // STORE_SETTINGS_REPOSITORY_H


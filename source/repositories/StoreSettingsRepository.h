#ifndef STORE_SETTINGS_REPOSITORY_H
#define STORE_SETTINGS_REPOSITORY_H

#include "../utils/FileHelper.h"
#include "../utils/PathHelper.h"
#include "../utils/StringHelper.h"
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
    }

public:
    StoreSettingsRepository(std::string path = "data/system/StoreSettings.txt")
        : filePath(path) {
        initDefaults();
    }

    const std::string& getFilePath() const { return filePath; }

    bool loadFromFile() {
        std::string resolved = PathHelper::resolveDataPath(filePath);
        std::ifstream file(resolved);
        if (!file.is_open()) {
            initDefaults();
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            size_t sep = line.find('|');
            if (sep != std::string::npos) {
                std::string k = StringHelper::trim(line.substr(0, sep));
                std::string v = StringHelper::trim(line.substr(sep + 1));
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
            out << "key|value\n";
            for (const auto& kv : settings) {
                out << kv.first << "|" << kv.second << "\n";
            }
            return true;
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
};

#endif // STORE_SETTINGS_REPOSITORY_H


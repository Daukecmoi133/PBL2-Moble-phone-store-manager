#ifndef REVIEW_MANAGER_H
#define REVIEW_MANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include "../../UI/viewmodels/ReviewViewModel.h"
#include "PathHelper.h"
#include "StringHelper.h"

/**
 * @file ReviewManager.h
 * @brief Quản lý lưu trữ và truy vấn đánh giá sản phẩm (Product Reviews) từ tệp dữ liệu
 */
class ReviewManager {
private:
    static std::string getFilePath() {
        std::string resolved = PathHelper::resolveDataPath("data/Device/ProductReviews.txt");
        if (resolved.empty()) {
            resolved = "data/Device/ProductReviews.txt";
        }
        return resolved;
    }

public:
    static std::vector<ProductReviewViewModel> loadAllReviews() {
        std::vector<ProductReviewViewModel> list;
        std::string path = getFilePath();
        std::ifstream file(path);
        if (!file.is_open()) {
            return list;
        }

        std::string line;
        bool isFirst = true;
        while (std::getline(file, line)) {
            line = StringHelper::trim(line);
            if (line.empty()) continue;
            if (isFirst && line.find("phoneId") != std::string::npos) {
                isFirst = false;
                continue;
            }
            isFirst = false;

            std::stringstream ss(line);
            std::string pId, cId, cName, starsStr, cmt, time;
            if (std::getline(ss, pId, '|') &&
                std::getline(ss, cId, '|') &&
                std::getline(ss, cName, '|') &&
                std::getline(ss, starsStr, '|') &&
                std::getline(ss, cmt, '|') &&
                std::getline(ss, time, '|')) {
                int stars = 5;
                try {
                    stars = std::stoi(starsStr);
                    if (stars < 1) stars = 1;
                    if (stars > 5) stars = 5;
                } catch (...) {
                    stars = 5;
                }
                list.push_back(ProductReviewViewModel(pId, cId, cName, stars, cmt, time));
            }
        }
        return list;
    }

    static std::vector<ProductReviewViewModel> getReviewsForProduct(const std::string& phoneId) {
        std::vector<ProductReviewViewModel> all = loadAllReviews();
        std::vector<ProductReviewViewModel> res;
        for (const auto& r : all) {
            if (r.phoneId == phoneId) {
                res.push_back(r);
            }
        }
        // Đảo ngược để bài mới nhất lên đầu
        std::reverse(res.begin(), res.end());
        return res;
    }

    static std::vector<ProductReviewViewModel> getReviewsForCustomer(const std::string& customerId) {
        std::vector<ProductReviewViewModel> all = loadAllReviews();
        std::vector<ProductReviewViewModel> res;
        for (const auto& r : all) {
            if (r.customerId == customerId) {
                res.push_back(r);
            }
        }
        std::reverse(res.begin(), res.end());
        return res;
    }

    static bool saveReview(const ProductReviewViewModel& review) {
        std::string path = getFilePath();
        std::ofstream file(path, std::ios::app);
        if (!file.is_open()) {
            return false;
        }
        file << review.phoneId << "|"
             << review.customerId << "|"
             << review.customerName << "|"
             << review.ratingStars << "|"
             << review.comment << "|"
             << review.reviewTime << "\n";
        return true;
    }

    static std::string getCurrentTimeString() {
        std::time_t now = std::time(nullptr);
        std::tm* tm_now = std::localtime(&now);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", tm_now);
        return std::string(buf);
    }
};

#endif // REVIEW_MANAGER_H


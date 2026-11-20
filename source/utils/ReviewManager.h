#ifndef REVIEW_MANAGER_H
#define REVIEW_MANAGER_H

#include "../../UI/viewmodels/ReviewViewModel.h"
#include "DateTime.h"
#include "FileHelper.h"
#include "PathHelper.h"
#include "SchemaHelper.h"
#include "StringHelper.h"
#include "TextCodec.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/**
 * @file ReviewManager.h
 * @brief Quản lý lưu trữ và truy vấn đánh giá sản phẩm (Product Reviews) từ tệp
 * dữ liệu
 */
class ReviewManager {
private:
  static std::string normalizeReviewTime(const std::string &rawTime) {
    DateTime dt(0, 0, 0, 0, 0, 0);
    if (DateTime::tryParse(rawTime, dt)) {
      return dt.toString();
    }
    return rawTime;
  }

public:
  static std::string getFilePath() {
    std::string resolved =
        PathHelper::resolveDataPath("data/Device/ProductReviews.txt");
    if (resolved.empty()) {
      resolved = "data/Device/ProductReviews.txt";
    }
    return resolved;
  }

  static std::vector<ProductReviewViewModel> loadAllReviews() {
    std::vector<ProductReviewViewModel> list;
    std::string path = getFilePath();
    std::ifstream file(path);
    if (!file.is_open()) {
      return list;
    }

    std::string line;
    bool isFirstNonEmpty = true;
    int lineNum = 0;
    while (std::getline(file, line)) {
      ++lineNum;
      if (lineNum == 1) StringHelper::stripUtf8Bom(line);
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (StringHelper::trim(line).empty()) continue;

      if (isFirstNonEmpty) {
        isFirstNonEmpty = false;
        auto kind = SchemaHelper::inspectFirstLine(
            line,
            SchemaHelper::PRODUCT_REVIEWS_HEADER,
            {},
            path,
            [](const std::string &l) {
              return SchemaHelper::looksLikePrefixedRecord(l, "PHN");
            });
        if (kind != SchemaHelper::FirstLineKind::DATA_ROW) {
          continue;
        }
      }

      Vector<std::string> parts = TextCodec::splitFields(line, '|');
      if (parts.getSize() >= 6) {
        std::string pId = StringHelper::trim(TextCodec::unescape(parts[0]));
        std::string cId = StringHelper::trim(TextCodec::unescape(parts[1]));
        std::string cName = TextCodec::unescape(parts[2]);
        std::string starsStr = StringHelper::trim(parts[3]);
        std::string cmt = TextCodec::unescape(parts[4]);
        std::string time = StringHelper::trim(TextCodec::unescape(parts[5]));

        if (pId.empty() || cId.empty()) continue;

        int stars = 5;
        try {
          stars = std::stoi(starsStr);
          if (stars < 1) stars = 1;
          if (stars > 5) stars = 5;
        } catch (...) {
          stars = 5;
        }
        list.push_back(
            ProductReviewViewModel(pId, cId, cName, stars, cmt, time));
      }
    }
    return list;
  }

  static bool saveAllReviews(const std::vector<ProductReviewViewModel> &all) {
    std::string path = getFilePath();
    return FileHelper::atomicWrite(path, [&](std::ofstream &file) {
      file << SchemaHelper::PRODUCT_REVIEWS_HEADER << "\n";
      for (const auto &r : all) {
        file << TextCodec::escapeField(r.phoneId) << "|"
             << TextCodec::escapeField(r.customerId) << "|"
             << TextCodec::escapeField(r.customerName) << "|"
             << r.ratingStars << "|"
             << TextCodec::escapeField(r.comment) << "|"
             << TextCodec::escapeField(normalizeReviewTime(r.reviewTime))
             << "\n";
      }
      return file.good();
    });
  }

  static std::vector<ProductReviewViewModel>
  getReviewsForProduct(const std::string &phoneId) {
    std::vector<ProductReviewViewModel> all = loadAllReviews();
    std::vector<ProductReviewViewModel> res;
    for (const auto &r : all) {
      if (r.phoneId == phoneId) {
        res.push_back(r);
      }
    }
    // Đảo ngược để bài mới nhất lên đầu
    std::reverse(res.begin(), res.end());
    return res;
  }

  static std::vector<ProductReviewViewModel>
  getReviewsForCustomer(const std::string &customerId) {
    std::vector<ProductReviewViewModel> all = loadAllReviews();
    std::vector<ProductReviewViewModel> res;
    for (const auto &r : all) {
      if (r.customerId == customerId) {
        res.push_back(r);
      }
    }
    std::reverse(res.begin(), res.end());
    return res;
  }

  static bool hasCustomerReviewedProduct(const std::string &phoneId,
                                         const std::string &customerId) {
    if (phoneId.empty() || customerId.empty())
      return false;
    std::vector<ProductReviewViewModel> all = loadAllReviews();
    for (const auto &r : all) {
      if (r.phoneId == phoneId && r.customerId == customerId) {
        return true;
      }
    }
    return false;
  }

  static ProductReviewViewModel
  getCustomerReview(const std::string &phoneId, const std::string &customerId) {
    if (phoneId.empty() || customerId.empty())
      return ProductReviewViewModel();
    std::vector<ProductReviewViewModel> all = loadAllReviews();
    for (const auto &r : all) {
      if (r.phoneId == phoneId && r.customerId == customerId) {
        return r;
      }
    }
    return ProductReviewViewModel();
  }

  static bool upsertReview(const ProductReviewViewModel &review) {
    std::vector<ProductReviewViewModel> all = loadAllReviews();
    bool found = false;
    for (auto &r : all) {
      if (r.phoneId == review.phoneId && r.customerId == review.customerId) {
        r.ratingStars = review.ratingStars;
        r.comment = review.comment;
        r.reviewTime = normalizeReviewTime(review.reviewTime);
        if (!review.customerName.empty())
          r.customerName = review.customerName;
        found = true;
        break;
      }
    }
    if (!found) {
      ProductReviewViewModel normalized = review;
      normalized.reviewTime = normalizeReviewTime(review.reviewTime);
      all.push_back(normalized);
    }

    return saveAllReviews(all);
  }

  static bool saveReview(const ProductReviewViewModel &review) {
    return upsertReview(review);
  }

  static std::string getCurrentTimeString() {
    std::time_t now = std::time(nullptr);
    std::tm *tm_now = std::localtime(&now);
    char buf[64];
    if (tm_now) {
      std::strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", tm_now);
      return std::string(buf);
    }
    return "01/01/2026 00:00:00";
  }
};

#endif // REVIEW_MANAGER_H

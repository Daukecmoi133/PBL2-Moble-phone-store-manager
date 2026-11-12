#ifndef REVIEW_VIEWMODEL_H
#define REVIEW_VIEWMODEL_H

#include <string>

/**
 * @file ReviewViewModel.h
 * @brief ViewModel lưu trữ đánh giá và phản hồi của khách hàng về sản phẩm
 */
struct ProductReviewViewModel {
    std::string phoneId;
    std::string customerId;
    std::string customerName;
    int ratingStars; // 1 đến 5 sao
    std::string comment;
    std::string reviewTime;

    ProductReviewViewModel() : ratingStars(5) {}
    ProductReviewViewModel(const std::string& pId, const std::string& cId,
                           const std::string& cName, int stars,
                           const std::string& cmt, const std::string& time)
        : phoneId(pId), customerId(cId), customerName(cName),
          ratingStars(stars), comment(cmt), reviewTime(time) {}
};

#endif // REVIEW_VIEWMODEL_H


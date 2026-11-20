#ifndef PRICING_CALCULATOR_H
#define PRICING_CALCULATOR_H

#include "../models/Common.h"
#include "StringHelper.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

/**
 * @file PricingCalculator.h
 * @brief Bộ tính giá, giảm giá và thuế VAT duy nhất dùng chung cho Giỏ hàng,
 *        Checkout, POS, Order, Invoice và Báo cáo (D1, AUD-02, AUD-12).
 *
 * Quy tắc chính thức:
 *  1. Tiền hàng (subtotal) tính trước VAT.
 *  2. Mã giảm giá (% hoặc số tiền cố định) giảm TRƯỚC trên subtotal (kẹp <= subtotal).
 *  3. Thuế suất VAT cấu hình trong StoreSettings.txt (mặc định 8% = 0.08) nhân trên
 *     phần tiền sau giảm giá: taxableAmount = max(0, subtotal - discount).
 *  4. Chỉ làm tròn MỘT LẦN ở kết quả cuối bằng std::llround, đồng thời đảm bảo
 *     nhất quán nguyên VND: subtotal - discountAmount + vatAmount == grandTotal.
 *  5. Chứng từ cũ trước khi áp dụng VAT giữ nguyên (vatApplied = false), KHÔNG tính lại.
 */
struct PricingBreakdown {
    double subtotal = 0.0;
    double discountAmount = 0.0;
    double taxableAmount = 0.0;
    double vatRate = 0.0;
    double vatAmount = 0.0;
    double grandTotal = 0.0;
    bool vatApplied = true;
};

class PricingCalculator {
public:
    static constexpr double DEFAULT_VAT_RATE = 0.08;

    static double sanitizeVatRate(double rate) {
        if (std::isnan(rate) || std::isinf(rate) || rate < 0.0) {
            return 0.0;
        }
        return std::min(rate, 1.0);
    }

    static bool tryParseVatRate(const std::string& raw, double& outRate) {
        std::string s = StringHelper::trim(raw);
        if (s.empty()) return false;

        bool hasPercent = false;
        if (s.back() == '%') {
            hasPercent = true;
            s.pop_back();
            s = StringHelper::trim(s);
        }
        if (s.empty()) return false;

        try {
            size_t idx = 0;
            double v = std::stod(s, &idx);
            if (idx != s.size() || std::isnan(v) || std::isinf(v) || v < 0.0 || v > 100.0) {
                return false;
            }
            if (hasPercent || v > 1.0) {
                outRate = v / 100.0;
            } else {
                outRate = v;
            }
            return outRate >= 0.0 && outRate <= 1.0;
        } catch (...) {
            return false;
        }
    }

    static std::string formatVatRateDisk(double rate) {
        const double r = sanitizeVatRate(rate);
        std::ostringstream oss;
        oss << std::fixed;
        oss.precision(6);
        oss << r;
        std::string s = oss.str();
        size_t dot = s.find('.');
        if (dot != std::string::npos) {
            while (!s.empty() && s.back() == '0') {
                s.pop_back();
            }
            if (!s.empty() && s.back() == '.') {
                s.pop_back();
            }
        }
        if (s.empty()) return "0";
        return s;
    }

    static std::string formatVatPercent(double rate) {
        const double pct = sanitizeVatRate(rate) * 100.0;
        long long roundedPct = std::llround(pct);
        if (std::fabs(pct - static_cast<double>(roundedPct)) < 1e-6) {
            return std::to_string(roundedPct) + "%";
        }
        std::ostringstream oss;
        oss << std::fixed;
        oss.precision(2);
        oss << pct;
        std::string s = oss.str();
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
        return s + "%";
    }

    static double computeRawDiscount(double subtotal,
                                     DiscountType type,
                                     double value,
                                     double maxDiscountValue = 0.0) {
        if (subtotal <= 0.0 || value <= 0.0) return 0.0;
        double raw = 0.0;
        if (type == DiscountType::PERCENTAGE) {
            raw = subtotal * (value / 100.0);
            if (maxDiscountValue > 0.0 && raw > maxDiscountValue) {
                raw = maxDiscountValue;
            }
        } else {
            raw = value;
        }
        if (raw < 0.0) raw = 0.0;
        if (raw > subtotal) raw = subtotal;
        return raw;
    }

    static PricingBreakdown compute(double subtotal, double rawDiscount, double vatRate) {
        PricingBreakdown pb;
        const double cleanSub = (subtotal > 0.0) ? subtotal : 0.0;
        const long long subInt = std::llround(cleanSub);
        const double clampedDiscRaw = std::clamp(rawDiscount, 0.0, cleanSub);
        const double rate = sanitizeVatRate(vatRate);
        const double taxableRaw = std::max(0.0, cleanSub - clampedDiscRaw);

        // Làm tròn MỘT lần ở kết quả tổng cuối bằng std::llround
        const long long grandTotalInt = std::llround(taxableRaw * (1.0 + rate));

        long long discInt = 0;
        long long taxableInt = 0;
        long long vatInt = 0;

        if (rate <= 0.0) {
            taxableInt = grandTotalInt;
            discInt = std::max(0LL, subInt - taxableInt);
            vatInt = 0;
        } else {
            discInt = std::clamp(std::llround(clampedDiscRaw), 0LL, subInt);
            taxableInt = std::max(0LL, subInt - discInt);
            vatInt = grandTotalInt - taxableInt;
            if (vatInt < 0) {
                vatInt = 0;
                taxableInt = grandTotalInt;
                discInt = std::max(0LL, subInt - taxableInt);
            }
        }

        pb.subtotal = static_cast<double>(subInt);
        pb.discountAmount = static_cast<double>(discInt);
        pb.taxableAmount = static_cast<double>(taxableInt);
        pb.vatRate = rate;
        pb.vatAmount = static_cast<double>(vatInt);
        pb.grandTotal = static_cast<double>(grandTotalInt);
        pb.vatApplied = true;
        return pb;
    }

    static PricingBreakdown computeWithVoucher(double subtotal,
                                               DiscountType type,
                                               double value,
                                               double maxDiscount,
                                               double vatRate) {
        const double rawDisc = computeRawDiscount(subtotal, type, value, maxDiscount);
        return compute(subtotal, rawDisc, vatRate);
    }

    static PricingBreakdown computeFromDiscountAmount(double subtotal,
                                                      double discountAmount,
                                                      double vatRate) {
        return compute(subtotal, discountAmount, vatRate);
    }

    static PricingBreakdown fromStored(double subtotal,
                                       double storedDiscount,
                                       double storedVatRate,
                                       double storedVatAmount,
                                       bool vatApplied) {
        PricingBreakdown pb;
        const long long subInt = std::llround(std::max(0.0, subtotal));
        const long long discInt = std::clamp(std::llround(std::max(0.0, storedDiscount)), 0LL, subInt);
        const long long taxableInt = std::max(0LL, subInt - discInt);
        const long long vatInt = vatApplied ? std::max(0LL, std::llround(storedVatAmount)) : 0LL;

        pb.subtotal = static_cast<double>(subInt);
        pb.discountAmount = static_cast<double>(discInt);
        pb.taxableAmount = static_cast<double>(taxableInt);
        pb.vatRate = vatApplied ? sanitizeVatRate(storedVatRate) : 0.0;
        pb.vatAmount = static_cast<double>(vatInt);
        pb.grandTotal = static_cast<double>(taxableInt + vatInt);
        pb.vatApplied = vatApplied;
        return pb;
    }

    static double computeNetRevenue(double subtotal, double discountAmount) {
        const long long subInt = std::llround(std::max(0.0, subtotal));
        const long long discInt = std::clamp(std::llround(std::max(0.0, discountAmount)), 0LL, subInt);
        return static_cast<double>(std::max(0LL, subInt - discInt));
    }

    static std::vector<double> allocateLineAmounts(const std::vector<double>& lineGrossAmounts,
                                                   double targetNetTotal) {
        const size_t n = lineGrossAmounts.size();
        std::vector<double> result(n, 0.0);
        if (n == 0) return result;

        double grossSum = 0.0;
        for (double g : lineGrossAmounts) {
            if (g > 0.0) grossSum += g;
        }

        double allocated = 0.0;
        for (size_t i = 0; i < n; ++i) {
            if (i + 1 == n) {
                result[i] = targetNetTotal - allocated;
            } else if (grossSum > 0.0) {
                const double positiveGross = (lineGrossAmounts[i] > 0.0) ? lineGrossAmounts[i] : 0.0;
                result[i] = targetNetTotal * (positiveGross / grossSum);
                allocated += result[i];
            } else {
                result[i] = (i == 0) ? targetNetTotal : 0.0;
                allocated += result[i];
            }
        }
        return result;
    }
};

#endif // PRICING_CALCULATOR_H

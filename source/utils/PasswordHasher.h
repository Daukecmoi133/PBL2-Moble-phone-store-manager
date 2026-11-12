#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

/**
 * @file PasswordHasher.h
 * @brief Băm và kiểm tra mật khẩu bằng thư viện OpenSSL (libcrypto) — KHÔNG tự thiết kế thuật toán.
 *
 * Thuật toán: PBKDF2-HMAC-SHA256 (PKCS5_PBKDF2_HMAC của OpenSSL), salt ngẫu nhiên 16 byte lấy từ
 * RAND_bytes (CSPRNG), khóa dẫn xuất 32 byte, 600.000 vòng lặp (khuyến nghị OWASP cho PBKDF2-SHA256).
 *
 * Định dạng chuỗi lưu trong file (không chứa '|', ',' hay ';' nên an toàn với parser hiện tại):
 *     pbkdf2-sha256$<số vòng lặp>$<salt hex>$<hash hex>
 * Số vòng lặp nằm trong chuỗi nên có thể tăng chi phí trong tương lai mà vẫn xác thực được bản ghi cũ.
 *
 * Tương thích dữ liệu cũ: file cũ lưu mật khẩu dạng rõ (không có tiền tố 'pbkdf2-sha256$').
 * Giá trị đó được nhận diện là "legacy plaintext" để xác thực một lần rồi nâng cấp lên hash,
 * thay vì làm mất tài khoản hoặc tự đặt lại mật khẩu.
 */
namespace PasswordHasher {

constexpr const char* kScheme = "pbkdf2-sha256";
constexpr int kIterations = 600000;
constexpr std::size_t kSaltBytes = 16;
constexpr std::size_t kHashBytes = 32;
// Chặn bản ghi bị sửa với số vòng lặp phi lý (làm treo ứng dụng khi đăng nhập)
constexpr long kMaxAcceptedIterations = 10000000;
constexpr long kMinAcceptedIterations = 1000;

namespace detail {

inline std::string toHex(const unsigned char* data, std::size_t length) {
    static const char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(length * 2);
    for (std::size_t i = 0; i < length; ++i) {
        out.push_back(digits[data[i] >> 4]);
        out.push_back(digits[data[i] & 0x0F]);
    }
    return out;
}

inline int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

inline bool fromHex(const std::string& text, std::vector<unsigned char>& out) {
    if (text.empty() || text.size() % 2 != 0) return false;
    out.clear();
    out.reserve(text.size() / 2);
    for (std::size_t i = 0; i < text.size(); i += 2) {
        const int hi = hexValue(text[i]);
        const int lo = hexValue(text[i + 1]);
        if (hi < 0 || lo < 0) return false;
        out.push_back(static_cast<unsigned char>((hi << 4) | lo));
    }
    return true;
}

struct Parsed {
    long iterations = 0;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

// Phân tích "pbkdf2-sha256$iter$salthex$hashhex"; false nếu không đúng định dạng
inline bool parse(const std::string& stored, Parsed& out) {
    const std::string prefix = std::string(kScheme) + "$";
    if (stored.compare(0, prefix.size(), prefix) != 0) return false;

    const std::size_t iterEnd = stored.find('$', prefix.size());
    if (iterEnd == std::string::npos) return false;
    const std::size_t saltEnd = stored.find('$', iterEnd + 1);
    if (saltEnd == std::string::npos) return false;
    if (stored.find('$', saltEnd + 1) != std::string::npos) return false;

    const std::string iterText = stored.substr(prefix.size(), iterEnd - prefix.size());
    if (iterText.empty() || iterText.size() > 9) return false;
    for (char c : iterText) {
        if (c < '0' || c > '9') return false;
    }
    out.iterations = std::strtol(iterText.c_str(), nullptr, 10);
    if (out.iterations < kMinAcceptedIterations || out.iterations > kMaxAcceptedIterations) return false;

    if (!fromHex(stored.substr(iterEnd + 1, saltEnd - iterEnd - 1), out.salt)) return false;
    if (!fromHex(stored.substr(saltEnd + 1), out.hash)) return false;
    return out.salt.size() >= 8 && out.hash.size() == kHashBytes;
}

inline bool derive(const std::string& password, const std::vector<unsigned char>& salt,
                   long iterations, unsigned char* out, std::size_t outLength) {
    return PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
                             salt.data(), static_cast<int>(salt.size()),
                             static_cast<int>(iterations), EVP_sha256(),
                             static_cast<int>(outLength), out) == 1;
}

} // namespace detail

// true nếu chuỗi đã ở định dạng hash hợp lệ của lớp này
inline bool isHashed(const std::string& stored) {
    detail::Parsed parsed;
    return detail::parse(stored, parsed);
}

// Băm mật khẩu với salt ngẫu nhiên mới. Trả về chuỗi rỗng nếu OpenSSL báo lỗi
// (người gọi PHẢI coi chuỗi rỗng là lỗi, không được lưu mật khẩu dạng rõ thay thế).
inline std::string hash(const std::string& password) {
    if (password.empty()) return "";

    std::vector<unsigned char> salt(kSaltBytes);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) return "";

    unsigned char derived[kHashBytes];
    if (!detail::derive(password, salt, kIterations, derived, kHashBytes)) return "";

    std::string result = std::string(kScheme) + "$" + std::to_string(kIterations) + "$" +
                         detail::toHex(salt.data(), salt.size()) + "$" +
                         detail::toHex(derived, kHashBytes);
    OPENSSL_cleanse(derived, sizeof(derived));
    return result;
}

// Kiểm tra mật khẩu với chuỗi hash đã lưu (so sánh thời gian hằng số).
// Chỉ chấp nhận định dạng hash; chuỗi legacy plaintext KHÔNG khớp ở đây.
inline bool verify(const std::string& password, const std::string& stored) {
    if (password.empty()) return false;
    detail::Parsed parsed;
    if (!detail::parse(stored, parsed)) return false;

    unsigned char derived[kHashBytes];
    if (!detail::derive(password, parsed.salt, parsed.iterations, derived, kHashBytes)) return false;
    const bool match = CRYPTO_memcmp(derived, parsed.hash.data(), kHashBytes) == 0;
    OPENSSL_cleanse(derived, sizeof(derived));
    return match;
}

// true nếu hash hợp lệ nhưng dùng số vòng lặp thấp hơn mức hiện hành (nên băm lại sau khi đăng nhập đúng)
inline bool needsRehash(const std::string& stored) {
    detail::Parsed parsed;
    return detail::parse(stored, parsed) && parsed.iterations < kIterations;
}

// So sánh mật khẩu với giá trị legacy plaintext (chỉ dùng cho dữ liệu cũ chưa được chuyển đổi).
// Giá trị rỗng hoặc đã ở định dạng hash không bao giờ được coi là legacy plaintext.
inline bool matchesLegacyPlaintext(const std::string& password, const std::string& stored) {
    if (password.empty() || stored.empty() || isHashed(stored)) return false;
    if (password.size() != stored.size()) return false;
    return CRYPTO_memcmp(password.data(), stored.data(), stored.size()) == 0;
}

} // namespace PasswordHasher

#endif // PASSWORDHASHER_H


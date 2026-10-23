#include "FileStore.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

// Loại bỏ các ký tự khoảng trắng (' ', '\t', '\r', '\n') ở đầu và cuối chuỗi
std::string FileStore::trim(const std::string& str) {
    const std::string whitespace = " \t\r\n";
    const size_t first = str.find_first_not_of(whitespace);
    if (first == std::string::npos) return "";
    const size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, (last - first + 1));
}

// 1. Đọc nội dung file dạng bảng phân tách bằng ký tự '|'
std::vector<std::vector<std::string>> FileStore::docFile(const std::string& duongDan) {
    std::error_code ec;

    // Nếu file không tồn tại, trả về danh sách rỗng (không báo lỗi)
    if (!std::filesystem::exists(duongDan, ec)) {
        return {};
    }

    // Mở file để đọc dữ liệu
    std::ifstream file(duongDan);

    // Nếu file tồn tại nhưng không mở được, ném ngoại lệ có thông báo rõ đường dẫn
    if (!file.is_open()) {
        throw std::runtime_error("Không thể mở file để đọc: " + duongDan);
    }

    std::vector<std::vector<std::string>> ketQua;
    std::string dong;

    while (std::getline(file, dong)) {
        // Loại bỏ ký tự \r ở cuối dòng nếu file mang định dạng CRLF (Windows)
        if (!dong.empty() && dong.back() == '\r') {
            dong.pop_back();
        }

        // Bỏ qua dòng trống hoặc dòng chỉ chứa toàn khoảng trắng
        if (trim(dong).empty()) {
            continue;
        }

        // Tách các trường theo ký tự '|' và trim khoảng trắng từng trường
        std::vector<std::string> cacTruong;
        std::istringstream ss(dong);
        std::string truong;

        while (std::getline(ss, truong, '|')) {
            cacTruong.push_back(trim(truong));
        }

        // Nếu dòng kết thúc bằng '|', thêm trường rỗng ở cuối (ví dụ "A|B|" -> 3 trường)
        if (!dong.empty() && dong.back() == '|') {
            cacTruong.push_back("");
        }

        ketQua.push_back(cacTruong);
    }

    file.close();
    return ketQua;
}

// 2. Ghi file an toàn qua file tạm (.tmp) rồi đổi tên nguyên tử
bool FileStore::ghiFileAnToan(const std::string& duongDan, const std::vector<std::string>& cacDong) {
    const std::string tmpPath = duongDan + ".tmp";
    std::error_code ec;

    // Đảm bảo thư mục chứa file đã tồn tại
    std::filesystem::path target(duongDan);
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), ec);
    }

    // Mở file tạm để ghi nội dung
    std::ofstream outFile(tmpPath, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }

    // Ghi từng dòng dữ liệu kèm ký tự xuống dòng
    for (const auto& dong : cacDong) {
        outFile << dong << "\n";
    }

    // Flush bộ đệm ghi ra đĩa
    outFile.flush();

    // Kiểm tra trạng thái stream sau khi ghi và flush
    if (!outFile.good()) {
        outFile.close();
        std::filesystem::remove(tmpPath, ec);
        return false;
    }

    // Đóng file stream trước khi đổi tên (bắt buộc trên Windows để tránh file lock)
    outFile.close();
    if (outFile.fail()) {
        std::filesystem::remove(tmpPath, ec);
        return false;
    }

    // Đổi tên file tạm thành file gốc bằng std::filesystem::rename
    try {
        std::filesystem::rename(tmpPath, duongDan, ec);
        if (ec) {
            // Nếu đổi tên thất bại: xóa file tạm, giữ nguyên file gốc
            std::filesystem::remove(tmpPath, ec);
            return false;
        }
        return true;
    } catch (...) {
        // Nếu có ngoại lệ: xóa file tạm, giữ nguyên file gốc
        std::filesystem::remove(tmpPath, ec);
        return false;
    }
}

// 3. Kiểm tra trường: trả về false nếu chứa '|' hoặc ký tự xuống dòng
bool FileStore::kiemTraTruong(const std::string& giaTri) {
    return giaTri.find_first_of("|\r\n") == std::string::npos;
}

// 4. Nối danh sách các trường bằng ký tự '|'
std::string FileStore::ghepTruong(const std::vector<std::string>& cacTruong) {
    if (cacTruong.empty()) {
        return "";
    }
    std::string ketQua;
    for (size_t i = 0; i < cacTruong.size(); ++i) {
        if (i > 0) {
            ketQua += '|';
        }
        ketQua += cacTruong[i];
    }
    return ketQua;
}

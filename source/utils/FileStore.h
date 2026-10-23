#ifndef FILESTORE_H
#define FILESTORE_H

#include <string>
#include <vector>

/**
 * @file FileStore.h
 * @brief Lớp tiện ích đọc/ghi file dữ liệu phân tách bằng ký tự '|' (C++17).
 *        Hỗ trợ đọc dữ liệu an toàn, ghi nguyên tử qua file tạm (.tmp),
 *        kiểm tra và ghép trường dữ liệu.
 */
class FileStore {
public:
    // Lớp tiện ích chỉ chứa các phương thức tĩnh, không khởi tạo đối tượng
    FileStore() = delete;

    /**
     * @brief Đọc nội dung file dạng bảng phân tách bằng ký tự '|'.
     *        Bỏ qua dòng trống, loại bỏ khoảng trắng đầu/cuối của từng trường.
     * @param duongDan Đường dẫn tới file cần đọc.
     * @return Danh sách các dòng (vector<vector<string>>). Trả về danh sách rỗng nếu file không tồn tại.
     * @throws std::runtime_error Nếu file tồn tại nhưng không thể mở để đọc.
     */
    static std::vector<std::vector<std::string>> docFile(const std::string& duongDan);

    /**
     * @brief Ghi dữ liệu an toàn ra file (ghi ra [duongDan].tmp, flush, kiểm tra lỗi,
     *        sau đó đổi tên thành file gốc bằng std::filesystem::rename).
     *        Nếu xảy ra lỗi ở bất kỳ bước nào, file gốc được giữ nguyên và xóa file .tmp.
     * @param duongDan Đường dẫn file cần ghi.
     * @param cacDong Danh sách các dòng nội dung cần ghi.
     * @return true nếu ghi và đổi tên thành công; false nếu gặp lỗi.
     */
    static bool ghiFileAnToan(const std::string& duongDan, const std::vector<std::string>& cacDong);

    /**
     * @brief Kiểm tra trường dữ liệu trước khi ghi: không được chứa ký tự '|' hoặc ký tự xuống dòng.
     * @param giaTri Chuỗi giá trị cần kiểm tra.
     * @return true nếu hợp lệ; false nếu chứa '|', '\r' hoặc '\n'.
     */
    static bool kiemTraTruong(const std::string& giaTri);

    /**
     * @brief Ghép danh sách các trường thành một dòng chuỗi, phân tách nhau bằng ký tự '|'.
     * @param cacTruong Danh sách các trường dữ liệu cần ghép.
     * @return Chuỗi kết quả sau khi ghép bằng ký tự '|'.
     */
    static std::string ghepTruong(const std::vector<std::string>& cacTruong);

private:
    /**
     * @brief Hàm tiện ích nội bộ: loại bỏ các ký tự khoảng trắng ở đầu và cuối chuỗi.
     */
    static std::string trim(const std::string& str);
};

#endif // FILESTORE_H

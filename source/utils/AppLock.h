#ifndef APPLOCK_H
#define APPLOCK_H

#include <string>
#include <iostream>
#include <fstream>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <cstring>
#endif

/**
 * @class AppLock
 * @brief Khóa ứng dụng đơn phiên bản (Single-Instance Lock)
 *
 * Ngăn chặn hai hoặc nhiều tiến trình cùng chạy đồng thời và ghi vào thư mục data/,
 * tránh tình trạng ghi đè cạnh tranh (race condition) và làm hỏng file dữ liệu.
 *
 * - Trên Windows: Sử dụng CreateFile với dwShareMode = 0 (exclusive lock).
 * - Trên Linux: Sử dụng flock() với cờ LOCK_EX | LOCK_NB.
 *
 * Khóa tự động giải phóng khi đối tượng bị hủy (RAII) hoặc khi tiến trình kết thúc
 * (kể cả trong trường hợp crash do hệ điều hành tự động thu hồi handle/file descriptor).
 */
class AppLock {
private:
    std::string lockFilePath;
    bool locked;

#if defined(_WIN32)
    HANDLE fileHandle;
#else
    int fd;
#endif

public:
    explicit AppLock(const std::string& path)
        : lockFilePath(path),
          locked(false)
#if defined(_WIN32)
        , fileHandle(INVALID_HANDLE_VALUE)
#else
        , fd(-1)
#endif
    {}

    ~AppLock() {
        release();
    }

    // Không cho phép copy để đảm bảo tính duy nhất của lock
    AppLock(const AppLock&) = delete;
    AppLock& operator=(const AppLock&) = delete;

    // Cho phép move
    AppLock(AppLock&& other) noexcept
        : lockFilePath(std::move(other.lockFilePath)),
          locked(other.locked)
#if defined(_WIN32)
        , fileHandle(other.fileHandle)
#else
        , fd(other.fd)
#endif
    {
        other.locked = false;
#if defined(_WIN32)
        other.fileHandle = INVALID_HANDLE_VALUE;
#else
        other.fd = -1;
#endif
    }

    AppLock& operator=(AppLock&& other) noexcept {
        if (this != &other) {
            release();
            lockFilePath = std::move(other.lockFilePath);
            locked = other.locked;
#if defined(_WIN32)
            fileHandle = other.fileHandle;
            other.fileHandle = INVALID_HANDLE_VALUE;
#else
            fd = other.fd;
            other.fd = -1;
#endif
            other.locked = false;
        }
        return *this;
    }

    bool isLocked() const {
        return locked;
    }

    const std::string& getLockFilePath() const {
        return lockFilePath;
    }

    bool acquire(std::string& outErrorMessage) {
        if (locked) return true;

#if defined(_WIN32)
        // Mở hoặc tạo file lock với dwShareMode = 0 (không chia sẻ quyền đọc/ghi cho tiến trình khác)
        fileHandle = CreateFileA(
            lockFilePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, // Exclusive access
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (fileHandle == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_SHARING_VIOLATION || err == ERROR_ACCESS_DENIED) {
                outErrorMessage = "Ung dung PBL2 dang chay o mot tien trinh khac (file lock: " + lockFilePath + ").";
            } else {
                outErrorMessage = "Khong the tao file lock tai: " + lockFilePath + " (ma loi: " + std::to_string(err) + ").";
            }
            return false;
        }

        // Ghi PID hiện tại vào file để dễ chẩn đoán tiến trình nào đang giữ lock
        DWORD pid = GetCurrentProcessId();
        std::string pidStr = std::to_string(pid) + "\n";
        DWORD written = 0;
        SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN);
        SetEndOfFile(fileHandle);
        WriteFile(fileHandle, pidStr.data(), static_cast<DWORD>(pidStr.size()), &written, NULL);
        FlushFileBuffers(fileHandle);

        locked = true;
        return true;
#else
        fd = open(lockFilePath.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd < 0) {
            outErrorMessage = "Khong the tao file lock: " + lockFilePath + " (" + strerror(errno) + ").";
            return false;
        }

        if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                outErrorMessage = "Ung dung PBL2 dang chay o mot tien trinh khac (file lock: " + lockFilePath + ").";
            } else {
                outErrorMessage = "Loi khoa file lock: " + std::string(strerror(errno));
            }
            close(fd);
            fd = -1;
            return false;
        }

        pid_t pid = getpid();
        std::string pidStr = std::to_string(pid) + "\n";
        ftruncate(fd, 0);
        lseek(fd, 0, SEEK_SET);
        write(fd, pidStr.data(), pidStr.size());
        fsync(fd);

        locked = true;
        return true;
#endif
    }

    bool acquire() {
        std::string err;
        return acquire(err);
    }

    void release() {
        if (!locked) return;

#if defined(_WIN32)
        if (fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(fileHandle);
            fileHandle = INVALID_HANDLE_VALUE;
            // Xóa file lock khi thoát bình thường
            DeleteFileA(lockFilePath.c_str());
        }
#else
        if (fd >= 0) {
            flock(fd, LOCK_UN);
            close(fd);
            fd = -1;
            unlink(lockFilePath.c_str());
        }
#endif
        locked = false;
    }
};

#endif // APPLOCK_H

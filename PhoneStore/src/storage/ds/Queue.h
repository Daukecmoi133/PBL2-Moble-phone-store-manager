#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"
#include <stdexcept>

// Template : Cấu trúc hàng đợi FIFO (First In First Out) xây dựng dựa trên LinkedList
template <typename T>
class Queue {
private:
    LinkedList<T> list; // Danh sách liên kết cơ sở lưu trữ các phần tử

public:
    Queue(); // Hàm dựng mặc định
    ~Queue(); // Hàm hủy

    void push(const T& val); // Hàm đẩy phần tử vào cuối hàng đợi - lvalue
    void push(T&& val); // Hàm đẩy phần tử vào cuối hàng đợi - rvalue
    bool pop(); // Hàm loại bỏ phần tử ở đầu hàng đợi

    T& front(); // Hàm trả về phần tử ở đầu hàng đợi cho phép sửa
    const T& front() const; // Hàm trả về phần tử ở đầu hàng đợi chỉ đọc

    T& back(); // Hàm trả về phần tử ở cuối hàng đợi cho phép sửa
    const T& back() const; // Hàm trả về phần tử ở cuối hàng đợi chỉ đọc

    int getSize() const; // Hàm trả về kích thước hàng đợi
    bool isEmpty() const; // Hàm kiểm tra hàng đợi có rỗng không
    void clear(); // Hàm dọn sạch toàn bộ hàng đợi
};

// ================= IMPLEMENTATION =================
// Cấu trúc Queue lấy Linked list làm cơ sở -> các hàm chức năng đều được gọi về Linked list

template <typename T>
Queue<T>::Queue() = default; // Hàm lấy hàm dựng của Linked list làm mặc định

template <typename T>
Queue<T>::~Queue() = default; //  Hàm lấy hàm hủy của Linked list làm mặc định

template <typename T>
void Queue<T>::push(const T& val) {
    // Hàm đẩy phần tử vào cuối hàng đợi - lvalue
    list.push_back(val);
}

template <typename T>
void Queue<T>::push(T&& val) {
    // Hàm đẩy phần tử vào cuối hàng đợi - rvalue
    list.push_back(std::move(val));
}

template <typename T>
bool Queue<T>::pop() {
    // Hàm loại bỏ phần tử đầu tiên ra khỏi hàng đợi
    return list.pop_front();
}

template <typename T>
T& Queue<T>::front() {
    // Hàm trả về phần tử đầu hàng đợi - cho phép sửa
    if (list.isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    return list.get(0);
}

template <typename T>
const T& Queue<T>::front() const {
    // Hàm trả về phần tử đầu hàng đợi - không cho phép sửa
    if (list.isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    return list.get(0);
}

template <typename T>
T& Queue<T>::back() {
    // Hàm trả về phần tử cuối hàng đợi - cho phép sửa
    if (list.isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    return list.get(list.getSize() - 1);
}

template <typename T>
const T& Queue<T>::back() const {
    // Hàm trả về phần tử cuối hàng đợi - không cho phép sửa
    if (list.isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    return list.get(list.getSize() - 1);
}

template <typename T>
int Queue<T>::getSize() const {
    // Hàm trả về số lượng phần tử có trong hàng đợi
    return list.getSize();
}

template <typename T>
bool Queue<T>::isEmpty() const {
    // Hàm kiểm tra hàng đợi có rỗng không
    return list.isEmpty();
}

template <typename T>
void Queue<T>::clear() {
    // Hàm dọn sạch toàn bộ phần tử trong hàng đợi
    list.clear();
}

#endif // QUEUE_H

#ifndef STACK_H
#define STACK_H

#include "Vector.h"
#include <stdexcept>

// Template : Cấu trúc ngăn xếp LIFO (Last In First Out) xây dựng dựa trên Vector
template <typename T>
class Stack {
private:
    Vector<T> vec; // Vector cơ sở lưu trữ các phần tử trong ngăn xếp

public:
    Stack(); // Hàm dựng mặc định
    ~Stack(); // Hàm hủy mặc định

    void push(const T& val); // Hàm đẩy phần tử vào đỉnh ngăn xếp - lvalue
    void push(T&& val); // Hàm đẩy phần tử vào đỉnh ngăn xếp - rvalue
    bool pop(); // Hàm loại bỏ phần tử ở đỉnh ngăn xếp

    T& top(); // Hàm truy xuất phần tử ở đỉnh ngăn xếp - cho phép sửa
    const T& top() const; // Hàm truy xuất phần tử ở đỉnh ngăn xếp - chỉ đọc

    int getSize() const; // Hàm trả về kích thước hiện tại của ngăn xếp
    bool isEmpty() const; // Hàm kiểm tra ngăn xếp có rỗng không
    void clear(); // Hàm dọn sạch toàn bộ ngăn xếp
};

// ================= IMPLEMENTATION =================
// Cấu trúc Stack lấy Vector làm cơ sở -> các hàm chức năng đều được gọi về Vector

template <typename T>
Stack<T>::Stack() = default; // Hàm lấy hàm dựng của Vector làm mặc định

template <typename T>
Stack<T>::~Stack() = default; // Hàm lấy hàm hủy của Vector làm mặc định

template <typename T>
void Stack<T>::push(const T& val) {
    // Hàm đẩy phần tử vào đỉnh ngăn xếp - lvalue
    vec.push_back(val);
}

template <typename T>
void Stack<T>::push(T&& val) {
    // Hàm đẩy phần tử vào đỉnh ngăn xếp - rvalue
    vec.push_back(std::move(val));
}

template <typename T>
bool Stack<T>::pop() {
    // Hàm loại bỏ phần tử đỉnh ngăn xếp (trả về false nếu ngăn xếp rỗng)
    if (vec.isEmpty()) return false;
    vec.pop_back();
    return true;
}

template <typename T>
T& Stack<T>::top() {
    // Hàm trả về phần tử tại đỉnh ngăn xếp - cho phép sửa dữ liệu
    if (vec.isEmpty()) {
        throw std::runtime_error("Stack is empty");
    }
    return vec[vec.getSize() - 1];
}

template <typename T>
const T& Stack<T>::top() const {
    // Hàm trả về phần tử tại đỉnh ngăn xếp - không cho phép sửa dữ liệu
    if (vec.isEmpty()) {
        throw std::runtime_error("Stack is empty");
    }
    return vec[vec.getSize() - 1];
}

template <typename T>
int Stack<T>::getSize() const {
    // Hàm trả về số lượng phần tử có trong ngăn xếp
    return vec.getSize();
}

template <typename T>
bool Stack<T>::isEmpty() const {
    // Hàm kiểm tra ngăn xếp có rỗng hay không
    return vec.isEmpty();
}

template <typename T>
void Stack<T>::clear() {
    // Hàm xóa toàn bộ phần tử trong ngăn xếp
    vec.clear();
}

#endif // STACK_H

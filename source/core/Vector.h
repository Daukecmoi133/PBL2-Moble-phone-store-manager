#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <stdexcept>
#include <utility>

// Template : dùng để tái sử dụng cho nhiều kiểu dữ liệu khác nhau (mảng động)
template <typename T>
class Vector {
private:
    T* data;        // Con trỏ quản lý vùng nhớ động chứa các phần tử
    int size;       // Số lượng phần tử hiện có trong Vector
    int capacity;   // Sức chứa tối đa hiện tại trước khi cần cấp phát lại

    void resize(int newCapacity); // Hàm có chức năng thay đổi kích thước sức chứa của Vector

public:
    Vector(int initCapacity = 8); // Hàm dựng với sức chứa khởi tạo ban đầu
    Vector(const Vector& other); // Hàm sao chép sâu - lvalue
    Vector(Vector&& other) noexcept; // Hàm di chuyển vùng nhớ - rvalue
    ~Vector(); // Hàm hủy giải phóng bộ nhớ động

    Vector& operator=(const Vector& other); // Overloading toán tử gán sao chép - lvalue
    Vector& operator=(Vector&& other) noexcept; // Overloading toán tử gán di chuyển - rvalue

    void push_back(const T& item); // Hàm thêm phần tử vào cuối Vector - lvalue
    void push_back(T&& item); // Hàm thêm phần tử vào cuối Vector - rvalue
    void pop_back(); // Hàm xóa phần tử ở cuối Vector

    T& operator[](int index); // Overloading toán tử [] cho phép sửa dữ liệu
    const T& operator[](int index) const; // Overloading toán tử [] chỉ đọc dữ liệu

    T& at(int index); // Hàm truy xuất phần tử có kiểm tra biên (ném ngoại lệ out_of_range)
    const T& at(int index) const; // Hàm truy xuất phần tử chỉ đọc có kiểm tra biên

    bool removeAt(int index); // Hàm xóa phần tử tại vị trí index
    void clear(); // Hàm xóa toàn bộ dữ liệu trong Vector

    int getSize() const; // Hàm trả về số lượng phần tử có trong Vector
    int getCapacity() const; // Hàm trả về sức chứa của Vector
    bool isEmpty() const; // Hàm kiểm tra Vector có rỗng hay không

    T* begin(); // Hàm trả về con trỏ tới phần tử đầu tiên (cho phép sửa)
    const T* begin() const; // Hàm trả về con trỏ tới phần tử đầu tiên (chỉ đọc)
    T* end(); // Hàm trả về con trỏ tới vị trí sau phần tử cuối cùng (cho phép sửa)
    const T* end() const; // Hàm trả về con trỏ tới vị trí sau phần tử cuối cùng (chỉ đọc)
};

// ================= IMPLEMENTATION =================

template <typename T>
void Vector<T>::resize(int newCapacity) {
    // Hàm có chức năng thay đổi kích thước Vector 
    if (newCapacity < size) newCapacity = size;
    if (newCapacity <= 0) newCapacity = 1;
    
    T* newData = new T[newCapacity];
    for (int i = 0; i < size; ++i) {
        newData[i] = std::move(data[i]);
    }
    delete[] data;
    data = newData;
    capacity = newCapacity;
}

template <typename T>
Vector<T>::Vector(int initCapacity) 
    : size(0), capacity(initCapacity > 0 ? initCapacity : 8) {
    // Hàm dựng
    data = new T[capacity];
}

template <typename T>
Vector<T>::Vector(const Vector& other) 
    : size(other.size), capacity(other.capacity) {
    // Hàm sao chép - lvalue
    data = new T[capacity];
    for (int i = 0; i < size; ++i) {
        data[i] = other.data[i];
    }
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept 
    : data(other.data), size(other.size), capacity(other.capacity) {
    // Hàm sao chép - rvalue
    other.data = nullptr;
    other.size = 0;
    other.capacity = 0;
}

template <typename T>
Vector<T>::~Vector() {
    // Hàm hủy
    delete[] data;
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& other) {
    // Overloading cho "=" - lvalue
    if (this != &other) {
        delete[] data;
        size = other.size;
        capacity = other.capacity;
        data = new T[capacity];
        for (int i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) noexcept { // Dùng noexcept để tăng độ ưu tiên khi đa năng hóa
    // Overloading cho "=" - rvalue 
    if (this != &other) {
        delete[] data;
        data = other.data;
        size = other.size;
        capacity = other.capacity;

        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }
    return *this;
}

template <typename T>
void Vector<T>::push_back(const T& item) {
    // Hàm insert đúng tiêu chuẩn tên gọi - lvalue
    if (size == capacity) {
        resize(capacity * 2);
    }
    data[size++] = item;
}

template <typename T>
void Vector<T>::push_back(T&& item) {
    // Hàm insert đúng tiêu chuẩn tên gọi - rvalue
    if (size == capacity) {
        resize(capacity * 2);
    }
    data[size++] = std::move(item);
}

template <typename T>
void Vector<T>::pop_back() {
    // Hàm remove ở cuối list đúng tiêu chuẩn tên gọi
    if (size > 0) {
        --size;
    }
}

template <typename T>
T& Vector<T>::operator[](int index) {
    // Overloading cho "[]"
    // Hàm cho phép UPDATE dữ liệu trả về
    return data[index];
}

template <typename T>
const T& Vector<T>::operator[](int index) const {
    // Overloading cho "[]"
    // Hàm không cho phép UPDATE dữ liệu trả về
    return data[index];
}

template <typename T>
T& Vector<T>::at(int index) {
    // Hàm đúng tiêu chuẩn tên gọi
    if (index < 0 || index >= size) {
        throw std::out_of_range("Vector index out of range");
    }
    return data[index];
}

template <typename T>
const T& Vector<T>::at(int index) const {
    // Hàm đúng tiêu chuẩn tên gọi
    if (index < 0 || index >= size) {
        throw std::out_of_range("Vector index out of range");
    }
    return data[index];
}

template <typename T>
bool Vector<T>::removeAt(int index) {
    // Hàm đúng tiêu chuẩn tên gọi
    if (index < 0 || index >= size) {
        return false;
    }
    for (int i = index; i < size - 1; ++i) {
        data[i] = std::move(data[i + 1]);
    }
    --size;
    return true;
}

template <typename T>
void Vector<T>::clear() {
    // Hàm có chức năng xóa toàn bộ dữ liệu trong list
    size = 0;
}

template <typename T>
int Vector<T>::getSize() const {
    // Hàm có chức năng trả về số lượng phần tử có trong list
    return size;
}

template <typename T>
int Vector<T>::getCapacity() const {
    // Hàm trả về sức chứa của Vector
    return capacity;
}

template <typename T>
bool Vector<T>::isEmpty() const {
    // Hàm cho biết list có rỗng không
    return size == 0;
}

template <typename T>
T* Vector<T>::begin() {
    // Hàm trả về con trỏ head của list 
    // Hàm cho phép update dữ liệu đầu ra
    return data;
}

template <typename T>
const T* Vector<T>::begin() const {
    // Hàm trả về con trỏ head của list
    // Hàm không cho phép update dũ liệu đầu ra
    return data;
}

template <typename T>
T* Vector<T>::end() {
    // Hàm trả về con trỏ tail của list 
    // Hàm cho phép update dữ liệu đầu ra
    return data + size;
}

template <typename T>
const T* Vector<T>::end() const {
    // Hàm trả về con trỏ tail của list 
    // Hàm không cho phép update dữ liệu đầu ra
    return data + size;
}

#endif // VECTOR_H

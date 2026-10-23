#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <stdexcept>
#include <utility>

// Template : Cấu trúc Node trong danh sách liên kết đôi (Doubly Linked List)
template <typename T>
struct ListNode {
    T data;         // Dữ liệu lưu trữ trong Node
    ListNode* prev; // Con trỏ trỏ tới Node đứng trước
    ListNode* next; // Con trỏ trỏ tới Node đứng sau

    ListNode(const T& val); // Hàm tạo Node với dữ liệu lvalue
    ListNode(T&& val);      // Hàm tạo Node với dữ liệu rvalue (di chuyển vùng nhớ)
};

// Template : Danh sách liên kết đôi hỗ trợ thêm/xóa hai đầu và truy xuất theo chỉ mục
template <typename T>
class LinkedList {
private:
    ListNode<T>* head; // Con trỏ trỏ tới Node đầu danh sách
    ListNode<T>* tail; // Con trỏ trỏ tới Node cuối danh sách
    int size;          // Số lượng Node hiện có trong danh sách

public:
    LinkedList(); // Hàm dựng khởi tạo danh sách rỗng
    ~LinkedList(); // Hàm hủy giải phóng toàn bộ các Node
    LinkedList(const LinkedList& other); // Hàm sao chép sâu - lvalue
    LinkedList(LinkedList&& other) noexcept; // Hàm di chuyển quyền sở hữu vùng nhớ - rvalue

    LinkedList& operator=(const LinkedList& other); // Overloading toán tử gán sao chép - lvalue
    LinkedList& operator=(LinkedList&& other) noexcept; // Overloading toán tử gán di chuyển - rvalue

    void push_back(const T& val); // Hàm thêm phần tử vào cuối danh sách
    void push_front(const T& val); // Hàm thêm phần tử vào đầu danh sách
    bool pop_front(); // Hàm xóa phần tử ở đầu danh sách
    bool pop_back(); // Hàm xóa phần tử ở cuối danh sách

    T& get(int index); // Hàm truy xuất phần tử tại index cho phép sửa dữ liệu
    const T& get(int index) const; // Hàm truy xuất phần tử tại index chỉ đọc dữ liệu

    bool removeAt(int index); // Hàm xóa phần tử tại vị trí index
    void clear(); // Hàm dọn sạch toàn bộ danh sách liên kết

    int getSize() const; // Hàm trả về số lượng phần tử có trong danh sách
    bool isEmpty() const; // Hàm kiểm tra danh sách có rỗng hay không
    ListNode<T>* getHead() const; // Hàm trả về con trỏ head của danh sách
    ListNode<T>* getTail() const; // Hàm trả về con trỏ tail của danh sách
};

// ================= IMPLEMENTATION =================

template <typename T> // Hàm dưới dùng cho lvalue - copy dữ liệu - lấy địa chỉ
ListNode<T>::ListNode(const T& val) : data(val), prev(nullptr), next(nullptr) {}

template <typename T> // Hàm dưới dùng cho rvalue(giá trị tạm thời, sẽ biến mất sau khi gọi lệnh) - move data - lấy giá trị - đẩy nhanh hiệu suất
ListNode<T>::ListNode(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}

template <typename T> // Hàm dựng
LinkedList<T>::LinkedList() : head(nullptr), tail(nullptr), size(0) {}

template <typename T> //  Hàm hủy
LinkedList<T>::~LinkedList() {
    clear();
}

template <typename T>
LinkedList<T>::LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), size(0) {
    // Hàm sao chép 
    ListNode<T>* curr = other.head;
    while (curr != nullptr) {
        push_back(curr->data);
        curr = curr->next;
    }
}

template <typename T>
LinkedList<T>::LinkedList(LinkedList&& other) noexcept 
    : head(other.head), tail(other.tail), size(other.size) {
    // Hàm move -  là hàm sao chép nhưng thay vì copy dữ liệu, hàm chuyển quyền sở hữu bộ nhớ từ biến "đầu" sang biến "đích"
    other.head = nullptr;
    other.tail = nullptr;
    other.size = 0;
    // Rồi tước quyền sỡ hữu của biến "đầu" 
}

template <typename T>
LinkedList<T>& LinkedList<T>::operator=(const LinkedList& other) {
    // Overloading cho "=" - lvalue
    if (this != &other) {
        clear();
        ListNode<T>* curr = other.head;
        while (curr != nullptr) {
            push_back(curr->data);
            curr = curr->next;
        }
    }
    return *this;
}

template <typename T>
LinkedList<T>& LinkedList<T>::operator=(LinkedList&& other) noexcept {
    // Overloading cho "=" - rvalue
    if (this != &other) {
        clear();
        head = other.head;
        tail = other.tail;
        size = other.size;

        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }
    return *this;
}

template <typename T>
void LinkedList<T>::push_back(const T& val) {
    // Hàm insert vào cuối linked list
    ListNode<T>* newNode = new ListNode<T>(val);
    if (tail == nullptr) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    ++size;
}

template <typename T>
void LinkedList<T>::push_front(const T& val) {
    // Hàm insert vào đầu linked list
    ListNode<T>* newNode = new ListNode<T>(val);
    if (head == nullptr) {
        head = tail = newNode;
    } else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    ++size;
}

template <typename T>
bool LinkedList<T>::pop_front() {
    // Hàm remove đầu linked list
    if (head == nullptr) return false;
    ListNode<T>* temp = head;
    head = head->next;
    if (head != nullptr) {
        head->prev = nullptr;
    } else {
        tail = nullptr;
    }
    delete temp;
    --size;
    return true;
}

template <typename T>
bool LinkedList<T>::pop_back() {
    // Hàm remover vào cuối linked list
    if (tail == nullptr) return false;
    ListNode<T>* temp = tail;
    tail = tail->prev;
    if (tail != nullptr) {
        tail->next = nullptr;
    } else {
        head = nullptr;
    }
    delete temp;
    --size;
    return true;
}

template <typename T>
T& LinkedList<T>::get(int index) {
    // Hàm có chức năng trả về Node tại vị trí index
    // Hàm cho phép update dữ liệu
    if (index < 0 || index >= size) {
        throw std::out_of_range("Index out of range in LinkedList");
    }
    ListNode<T>* curr = head;
    for (int i = 0; i < index; ++i) {
        curr = curr->next;
    }
    return curr->data;
}

template <typename T>
const T& LinkedList<T>::get(int index) const {
    // Hàm có chức năng trả về Node tại vị trí index
    // Hàm không cho phép update dữ liệu
    if (index < 0 || index >= size) {
        throw std::out_of_range("Index out of range in LinkedList");
    }
    ListNode<T>* curr = head;
    for (int i = 0; i < index; ++i) {
        curr = curr->next;
    }
    return curr->data;
}

template <typename T>
bool LinkedList<T>::removeAt(int index) {
    // Hàm cho phép remove Node tại vị trí index
    if (index < 0 || index >= size) return false;
    if (index == 0) return pop_front();
    if (index == size - 1) return pop_back();

    ListNode<T>* curr = head;
    for (int i = 0; i < index; ++i) {
        curr = curr->next;
    }
    curr->prev->next = curr->next;
    curr->next->prev = curr->prev;
    delete curr;
    --size;
    return true;
}

template <typename T>
void LinkedList<T>::clear() {
    // Hàm có chức năng dọn toàn bộ Linked list
    ListNode<T>* curr = head;
    while (curr != nullptr) {
        ListNode<T>* next = curr->next;
        delete curr;
        curr = next;
    }
    head = tail = nullptr;
    size = 0;
}

template <typename T>
int LinkedList<T>::getSize() const {
    // Hàm trả về số lượng phần tử có trong Linked list
    return size;
}

template <typename T>
bool LinkedList<T>::isEmpty() const {
    // Hàm trả về liệu Linked list có rỗng không
    return size == 0;
}

template <typename T>
ListNode<T>* LinkedList<T>::getHead() const {
    // Hàm trả về con trỏ head của Linked list
    return head;
}

template <typename T>
ListNode<T>* LinkedList<T>::getTail() const {
    // Hàm trả về con trỏ tail của Linked list
    return tail;
}

#endif // LINKEDLIST_H

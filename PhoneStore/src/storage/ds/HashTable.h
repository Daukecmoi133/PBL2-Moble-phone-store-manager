#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <string>
#include <stdexcept>

// Hàm băm mặc định cho int và std::string - overloading
template <typename K>
struct HashFunction {
    unsigned long operator()(const K& key) const;
};

template <>
struct HashFunction<std::string> {
    unsigned long operator()(const std::string& str) const;
};

// ================= IMPLEMENTATION =================

template <typename K>
inline unsigned long HashFunction<K>::operator()(const K& key) const {
    return static_cast<unsigned long>(key);
}

inline unsigned long HashFunction<std::string>::operator()(const std::string& str) const {
    // Thuật toán băm chuỗi DJB2
    unsigned long hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}

// Template : dùng để tái sử dụng cho nhiều kiểu dữ liệu khác nhau
template <typename K, typename V, typename Hash = HashFunction<K>>
class HashTable {
private:
    struct HashNode {
        K key;
        V value;
        HashNode* next;

        HashNode(const K& k, const V& v);
    };

    HashNode** table;
    int capacity;
    int size;
    Hash hasher;

    int hashIndex(const K& key) const; 
    void rehash(); 

public:
    HashTable(int initCapacity = 17); // Hàm dựng 
    ~HashTable(); // Hàm hủy

    // Không cho phép copy nông để tránh lỗi dangling pointer - con trỏ trỏ về dữ liệu không tồn tại / hợp lệ gây giá trị rác / crash chương trình
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(const K& key, const V& value); // Hàm insert đúng tiêu chuẩn tên gọi
    V* find(const K& key); // Hàm find đúng tiêu chuẩn tên gọi -  sử dụng chạy tuần tự - Hàm cho phép sửa HashTable
    const V* find(const K& key) const; // Hàm find đúng tiêu chuẩn tên gọi -  sử dụng chạy tuần tự - // Hàm không cho phép sử HashTabe
    bool contains(const K& key) const; // Hàm có chức năng kiểm tra sự tồn tại của key
    bool remove(const K& key); // Hàm có chức năng xóa Node ra khỏi HashTable
    void clear();  // Hàm có chức năng giải phóng toàn bộ HashTable -> trả về 1 HashTable rỗng

    int getSize() const; // Hàm có chức năng trả về số lượng Node hiện tại có trong HashTable
    int getCapacity() const;  // Hàm có chức năng trả về sức chứa của HashTable
    bool isEmpty() const;  // Hàm trả về HashTable có rỗng hay không
};

// ================= IMPLEMENTATION HASHTABLE =================

template <typename K, typename V, typename Hash>
HashTable<K, V, Hash>::HashNode::HashNode(const K& k, const V& v) // Hàm tạo Node mới
    : key(k), value(v), next(nullptr) {}

template <typename K, typename V, typename Hash>
int HashTable<K, V, Hash>::hashIndex(const K& key) const { 
    // Hàm lấy vị trí của key trong HashTable
    return static_cast<int>(hasher(key) % capacity);
}

template <typename K, typename V, typename Hash>
void HashTable<K, V, Hash>::rehash() {
    // Hàm có chức năng thay đổi kích thức HashTable
    int oldCapacity = capacity;
    HashNode** oldTable = table;

    capacity = capacity * 2 + 1;
    table = new HashNode*[capacity]();
    size = 0;

    for (int i = 0; i < oldCapacity; ++i) {
        HashNode* curr = oldTable[i];
        while (curr != nullptr) {
            insert(curr->key, curr->value);
            HashNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
    delete[] oldTable;
}

template <typename K, typename V, typename Hash>
HashTable<K, V, Hash>::HashTable(int initCapacity) 
    : capacity(initCapacity > 0 ? initCapacity : 17), size(0) {
    table = new HashNode*[capacity]();
}

template <typename K, typename V, typename Hash>
HashTable<K, V, Hash>::~HashTable() {
    clear();
    delete[] table;
}

template <typename K, typename V, typename Hash>
void HashTable<K, V, Hash>::insert(const K& key, const V& value) {
    if (static_cast<double>(size + 1) / capacity > 0.75) {
        // LoadFactor = 0.75 là ngưỡng an toàn đảm bảo về tốc độ truy xuất - bộ nhớ 
        rehash();
    }

    int index = hashIndex(key);
    HashNode* curr = table[index];

    while (curr != nullptr) {
        if (curr->key == key) {
            // Cập nhật nếu đã tồn tại
            curr->value = value; 
            return;
        }
        curr = curr->next;
    }

    HashNode* newNode = new HashNode(key, value);
    newNode->next = table[index];
    table[index] = newNode;
    ++size;
}

template <typename K, typename V, typename Hash>
V* HashTable<K, V, Hash>::find(const K& key) {
    // Hàm tìm kiếm theo key - cho phép sửa value của phần tử tìm thấy
    int index = hashIndex(key);
    HashNode* curr = table[index];

    while (curr != nullptr) {
        if (curr->key == key) {
            return &(curr->value);
        }
        curr = curr->next;
    }
    return nullptr; // Không tìm thấy
}

template <typename K, typename V, typename Hash>
const V* HashTable<K, V, Hash>::find(const K& key) const {
    // Hàm tìm kiếm theo key - không cho phép sửa value (chỉ đọc)
    int index = hashIndex(key);
    HashNode* curr = table[index];

    while (curr != nullptr) {
        if (curr->key == key) {
            return &(curr->value);
        }
        curr = curr->next;
    }
    return nullptr; // Không tìm thấy
}

template <typename K, typename V, typename Hash>
bool HashTable<K, V, Hash>::contains(const K& key) const {
    // Hàm kiểm tra sự tồn tại của key trong bảng băm
    return find(key) != nullptr;
}

template <typename K, typename V, typename Hash>
bool HashTable<K, V, Hash>::remove(const K& key) {
    // Hàm xóa phần tử theo key khỏi bảng băm
    int index = hashIndex(key);
    HashNode* curr = table[index];
    HashNode* prev = nullptr;

    while (curr != nullptr) {
        if (curr->key == key) {
            if (prev == nullptr) {
                // Xóa node đầu của danh sách liên kết tại bucket
                table[index] = curr->next;
            } else {
                // Bỏ qua node hiện tại
                prev->next = curr->next;
            }
            delete curr;
            --size;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false; // Không tìm thấy key để xóa
}

template <typename K, typename V, typename Hash>
void HashTable<K, V, Hash>::clear() {
    // Hàm giải phóng toàn bộ các Node và dọn sạch bảng băm
    for (int i = 0; i < capacity; ++i) {
        HashNode* curr = table[i];
        while (curr != nullptr) {
            HashNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
        table[i] = nullptr;
    }
    size = 0;
}

template <typename K, typename V, typename Hash>
int HashTable<K, V, Hash>::getSize() const {
    // Hàm trả về số lượng Node hiện có trong bảng băm
    return size;
}

template <typename K, typename V, typename Hash>
int HashTable<K, V, Hash>::getCapacity() const {
    // Hàm trả về sức chứa hiện tại của bảng băm
    return capacity;
}

template <typename K, typename V, typename Hash>
bool HashTable<K, V, Hash>::isEmpty() const {
    // Hàm kiểm tra bảng băm có rỗng không
    return size == 0;
}

#endif // HASHTABLE_H

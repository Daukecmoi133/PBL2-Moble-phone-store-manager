#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <string>
#include <stdexcept>

// Hàm băm mặc định cho int và std::string
template <typename K>
struct HashFunction {
    unsigned long operator()(const K& key) const {
        return static_cast<unsigned long>(key);
    }
};

template <>
struct HashFunction<std::string> {
    unsigned long operator()(const std::string& str) const {
        // Thuật toán băm chuỗi DJB2
        unsigned long hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
        }
        return hash;
    }
};

template <typename K, typename V, typename Hash = HashFunction<K>>
class HashTable {
private:
    struct HashNode {
        K key;
        V value;
        HashNode* next;

        HashNode(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    HashNode** table;
    int capacity;
    int size;
    Hash hasher;

    int hashIndex(const K& key) const {
        return static_cast<int>(hasher(key) % capacity);
    }

    void rehash() {
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

public:
    HashTable(int initCapacity = 17) : capacity(initCapacity > 0 ? initCapacity : 17), size(0) {
        table = new HashNode*[capacity]();
    }

    ~HashTable() {
        clear();
        delete[] table;
    }

    // Không cho phép copy nông để tránh lỗi dangling pointer
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(const K& key, const V& value) {
        if (static_cast<double>(size + 1) / capacity > 0.75) {
            rehash();
        }

        int index = hashIndex(key);
        HashNode* curr = table[index];

        while (curr != nullptr) {
            if (curr->key == key) {
                curr->value = value; // Cập nhật nếu đã tồn tại
                return;
            }
            curr = curr->next;
        }

        HashNode* newNode = new HashNode(key, value);
        newNode->next = table[index];
        table[index] = newNode;
        ++size;
    }

    V* find(const K& key) {
        int index = hashIndex(key);
        HashNode* curr = table[index];

        while (curr != nullptr) {
            if (curr->key == key) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    const V* find(const K& key) const {
        int index = hashIndex(key);
        HashNode* curr = table[index];

        while (curr != nullptr) {
            if (curr->key == key) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    bool contains(const K& key) const {
        return find(key) != nullptr;
    }

    bool remove(const K& key) {
        int index = hashIndex(key);
        HashNode* curr = table[index];
        HashNode* prev = nullptr;

        while (curr != nullptr) {
            if (curr->key == key) {
                if (prev == nullptr) {
                    table[index] = curr->next;
                } else {
                    prev->next = curr->next;
                }
                delete curr;
                --size;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    void clear() {
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

    int getSize() const { return size; }
    int getCapacity() const { return capacity; }
    bool isEmpty() const { return size == 0; }
};

#endif // HASHTABLE_H

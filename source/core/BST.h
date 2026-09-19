#ifndef BST_H
#define BST_H

#include <iostream>
#include "Vector.h"

// Template : dùng để tái sử dụng cho nhiều kiểu dữ liệu khác nhau
template <typename Key, typename Value>
class BST {
private:
    struct BSTNode {
        Key key;
        Value value;
        BSTNode* left;
        BSTNode* right;

        BSTNode(const Key& k, const Value& v);
    };

    BSTNode* root;
    int size;

    BSTNode* insertHelper(BSTNode* node, const Key& key, const Value& value); // Hàm có chức năng cập nhật cây khi có node mới
    BSTNode* findMin(BSTNode* node) const; // Hàm trả về min của BST
    BSTNode* removeHelper(BSTNode* node, const Key& key, bool& removed); //  Hàm có chức năng cập nhật cây sau khi xóa
    void inOrderHelper(BSTNode* node, Vector<Value>& result) const; // Hàm có chức năng chuyển BST về 1 Vector không giảm dần Value
    void rangeHelper(BSTNode* node, const Key& minKey, const Key& maxKey, Vector<Value>& result) const; // Hàm có chức năng lấy 1 Vector các Value trong khoảng cho trước
    void clearHelper(BSTNode* node);// Hàm có chức năng giải phóng 1 BST

public:
    BST();// Hàm dựng không tham số
    ~BST();// Hàm hủy

    // Không cho copy nông - copy phải dùng deep func tránh dùng cùng 1 vùng nhớ
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    void insert(const Key& key, const Value& value); //Hàm insert đúng quy chuẩn tên gọi
    Value* search(const Key& key); // Hàm search BST cho phép sửa dữ liệu BST
    const Value* search(const Key& key) const; // Hàm search BST không cho phép sửa dữ liệu BST
    bool remove(const Key& key); // Hàm remove dúng tiêu chuẩn tên gọi
    Vector<Value> toSortedVector() const; // Hàm lấy 1 Vector gồm các Value của BST theo GreaterSort
    Vector<Value> findRange(const Key& minKey, const Key& maxKey) const; // Hàm lấy Vector Value theo khoảng đúng tiêu chuẩn tên gọi
    void clear();// Hàm clear đúng tiêu chuẩn tên gọi

    int getSize() const;// Hàm trả về kích thước BST
    bool isEmpty() const;// Hàm trả về liệu BST có rỗng không
};

// ================= IMPLEMENTATION =================

template <typename Key, typename Value> // Cho func bên dưới biết Key và Value ở đâu ra
BST<Key, Value>::BSTNode::BSTNode(const Key& k, const Value& v) // Hàm tạo node mới
    : key(k), value(v), left(nullptr), right(nullptr) {}

template <typename Key, typename Value>
typename BST<Key, Value>::BSTNode* BST<Key, Value>::insertHelper(BSTNode* node, const Key& key, const Value& value) {
    

    if (node == nullptr) {
        ++size;
        return new BSTNode(key, value);
    }
    if (key < node->key) { 
        // sang trái
        node->left = insertHelper(node->left, key, value);
    } else if (key > node->key) { 
        // sang phải
        node->right = insertHelper(node->right, key, value);
    } else {
        // Nếu trùng key, cập nhật giá trị
        node->value = value;
    }
    return node;
}

template <typename Key, typename Value>
typename BST<Key, Value>::BSTNode* BST<Key, Value>::findMin(BSTNode* node) const { 
    while (node && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

template <typename Key, typename Value>
typename BST<Key, Value>::BSTNode* BST<Key, Value>::removeHelper(BSTNode* node, const Key& key, bool& removed) {  
    
    if (node == nullptr) return nullptr;

    if (key < node->key) {
        node->left = removeHelper(node->left, key, removed);
    } else if (key > node->key) {
        node->right = removeHelper(node->right, key, removed);
    } else {
        removed = true;
        // Trường hợp 0 hoặc 1 con
        if (node->left == nullptr) {
            BSTNode* temp = node->right;
            delete node;
            --size;
            return temp;
        } else if (node->right == nullptr) {
            BSTNode* temp = node->left;
            delete node;
            --size;
            return temp;
        }
        // Trường hợp 2 con: tìm node nhỏ nhất bên cây con phải
        BSTNode* temp = findMin(node->right);
        node->key = temp->key;
        node->value = temp->value;
        node->right = removeHelper(node->right, temp->key, removed);
    }
    return node;
}

template <typename Key, typename Value>
void BST<Key, Value>::inOrderHelper(BSTNode* node, Vector<Value>& result) const { 
    
    if (node == nullptr) return;
    inOrderHelper(node->left, result);
    result.push_back(node->value);
    inOrderHelper(node->right, result);
}

template <typename Key, typename Value>
void BST<Key, Value>::rangeHelper(BSTNode* node, const Key& minKey, const Key& maxKey, Vector<Value>& result) const { 
    
    if (node == nullptr) return;
    if (minKey < node->key) {
        rangeHelper(node->left, minKey, maxKey, result);
    }
    if (minKey <= node->key && node->key <= maxKey) {
        result.push_back(node->value);
    }
    if (maxKey > node->key) {
        rangeHelper(node->right, minKey, maxKey, result);
    }
}

template <typename Key, typename Value>
void BST<Key, Value>::clearHelper(BSTNode* node) { 
    
    if (node == nullptr) return;
    clearHelper(node->left);
    clearHelper(node->right);
    delete node;
}

template <typename Key, typename Value>
BST<Key, Value>::BST() : root(nullptr), size(0) {}


template <typename Key, typename Value>
BST<Key, Value>::~BST() {
    
    clear();
}

template <typename Key, typename Value>
void BST<Key, Value>::insert(const Key& key, const Value& value) {
    root = insertHelper(root, key, value);
}

template <typename Key, typename Value>
Value* BST<Key, Value>::search(const Key& key) {
    BSTNode* curr = root;
    while (curr != nullptr) {
        if (key == curr->key) {
            return &(curr->value);
        } else if (key < curr->key) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return nullptr;
}

template <typename Key, typename Value>
const Value* BST<Key, Value>::search(const Key& key) const {
    BSTNode* curr = root;
    while (curr != nullptr) {
        if (key == curr->key) {
            return &(curr->value);
        } else if (key < curr->key) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return nullptr;
}

template <typename Key, typename Value>
bool BST<Key, Value>::remove(const Key& key) {
    bool removed = false;
    root = removeHelper(root, key, removed);
    return removed;
}

template <typename Key, typename Value>
Vector<Value> BST<Key, Value>::toSortedVector() const {
    Vector<Value> result;
    inOrderHelper(root, result);
    return result;
}

template <typename Key, typename Value>
Vector<Value> BST<Key, Value>::findRange(const Key& minKey, const Key& maxKey) const {
    Vector<Value> result;
    rangeHelper(root, minKey, maxKey, result);
    return result;
}

template <typename Key, typename Value>
void BST<Key, Value>::clear() {
    
    clearHelper(root);
    root = nullptr;
    size = 0;
}

template <typename Key, typename Value>
int BST<Key, Value>::getSize() const {
    
    return size;
}

template <typename Key, typename Value>
bool BST<Key, Value>::isEmpty() const {
    
    return size == 0;
}

#endif // BST_H

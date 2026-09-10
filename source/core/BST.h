#ifndef BST_H
#define BST_H

#include <iostream>
#include "Vector.h"

template <typename Key, typename Value>
class BST {
private:
    struct BSTNode {
        Key key;
        Value value;
        BSTNode* left;
        BSTNode* right;

        BSTNode(const Key& k, const Value& v) 
            : key(k), value(v), left(nullptr), right(nullptr) {}
    };

    BSTNode* root;
    int size;

    BSTNode* insertHelper(BSTNode* node, const Key& key, const Value& value) {
        if (node == nullptr) {
            ++size;
            return new BSTNode(key, value);
        }
        if (key < node->key) {
            node->left = insertHelper(node->left, key, value);
        } else if (key > node->key) {
            node->right = insertHelper(node->right, key, value);
        } else {
            // Nếu trùng key, cập nhật giá trị
            node->value = value;
        }
        return node;
    }

    BSTNode* findMin(BSTNode* node) const {
        while (node && node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    BSTNode* removeHelper(BSTNode* node, const Key& key, bool& removed) {
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

    void inOrderHelper(BSTNode* node, Vector<Value>& result) const {
        if (node == nullptr) return;
        inOrderHelper(node->left, result);
        result.push_back(node->value);
        inOrderHelper(node->right, result);
    }

    void rangeHelper(BSTNode* node, const Key& minKey, const Key& maxKey, Vector<Value>& result) const {
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

    void clearHelper(BSTNode* node) {
        if (node == nullptr) return;
        clearHelper(node->left);
        clearHelper(node->right);
        delete node;
    }

public:
    BST() : root(nullptr), size(0) {}

    ~BST() {
        clear();
    }

    // Không cho copy nông
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    void insert(const Key& key, const Value& value) {
        root = insertHelper(root, key, value);
    }

    Value* search(const Key& key) {
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

    const Value* search(const Key& key) const {
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

    bool remove(const Key& key) {
        bool removed = false;
        root = removeHelper(root, key, removed);
        return removed;
    }

    Vector<Value> toSortedVector() const {
        Vector<Value> result;
        inOrderHelper(root, result);
        return result;
    }

    Vector<Value> findRange(const Key& minKey, const Key& maxKey) const {
        Vector<Value> result;
        rangeHelper(root, minKey, maxKey, result);
        return result;
    }

    void clear() {
        clearHelper(root);
        root = nullptr;
        size = 0;
    }

    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
};

#endif // BST_H

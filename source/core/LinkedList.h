#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T>
struct ListNode {
    T data;
    ListNode* prev;
    ListNode* next;

    ListNode(const T& val) : data(val), prev(nullptr), next(nullptr) {}
    ListNode(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}
};

template <typename T>
class LinkedList {
private:
    ListNode<T>* head;
    ListNode<T>* tail;
    int size;

public:
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // Destructor
    ~LinkedList() {
        clear();
    }

    // Copy Constructor
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), size(0) {
        ListNode<T>* curr = other.head;
        while (curr != nullptr) {
            push_back(curr->data);
            curr = curr->next;
        }
    }

    // Move Constructor
    LinkedList(LinkedList&& other) noexcept : head(other.head), tail(other.tail), size(other.size) {
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    // Copy Assignment
    LinkedList& operator=(const LinkedList& other) {
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

    // Move Assignment
    LinkedList& operator=(LinkedList&& other) noexcept {
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

    void push_back(const T& val) {
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

    void push_front(const T& val) {
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

    bool pop_front() {
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

    bool pop_back() {
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

    T& get(int index) {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Index out of range in LinkedList");
        }
        ListNode<T>* curr = head;
        for (int i = 0; i < index; ++i) {
            curr = curr->next;
        }
        return curr->data;
    }

    const T& get(int index) const {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Index out of range in LinkedList");
        }
        ListNode<T>* curr = head;
        for (int i = 0; i < index; ++i) {
            curr = curr->next;
        }
        return curr->data;
    }

    bool removeAt(int index) {
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

    void clear() {
        ListNode<T>* curr = head;
        while (curr != nullptr) {
            ListNode<T>* next = curr->next;
            delete curr;
            curr = next;
        }
        head = tail = nullptr;
        size = 0;
    }

    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }
    ListNode<T>* getHead() const { return head; }
    ListNode<T>* getTail() const { return tail; }
};

#endif // LINKEDLIST_H

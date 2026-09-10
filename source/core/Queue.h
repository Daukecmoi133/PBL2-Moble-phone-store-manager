#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"
#include <stdexcept>

template <typename T>
class Queue {
private:
    LinkedList<T> list;

public:
    Queue() = default;
    ~Queue() = default;

    void push(const T& val) {
        list.push_back(val);
    }

    void push(T&& val) {
        list.push_back(std::move(val));
    }

    bool pop() {
        return list.pop_front();
    }

    T& front() {
        if (list.isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        return list.get(0);
    }

    const T& front() const {
        if (list.isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        return list.get(0);
    }

    T& back() {
        if (list.isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        return list.get(list.getSize() - 1);
    }

    const T& back() const {
        if (list.isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        return list.get(list.getSize() - 1);
    }

    int getSize() const {
        return list.getSize();
    }

    bool isEmpty() const {
        return list.isEmpty();
    }

    void clear() {
        list.clear();
    }
};

#endif // QUEUE_H

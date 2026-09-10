#ifndef STACK_H
#define STACK_H

#include "Vector.h"
#include <stdexcept>

template <typename T>
class Stack {
private:
    Vector<T> vec;

public:
    Stack() = default;
    ~Stack() = default;

    void push(const T& val) {
        vec.push_back(val);
    }

    void push(T&& val) {
        vec.push_back(std::move(val));
    }

    bool pop() {
        if (vec.isEmpty()) return false;
        vec.pop_back();
        return true;
    }

    T& top() {
        if (vec.isEmpty()) {
            throw std::runtime_error("Stack is empty");
        }
        return vec[vec.getSize() - 1];
    }

    const T& top() const {
        if (vec.isEmpty()) {
            throw std::runtime_error("Stack is empty");
        }
        return vec[vec.getSize() - 1];
    }

    int getSize() const {
        return vec.getSize();
    }

    bool isEmpty() const {
        return vec.isEmpty();
    }

    void clear() {
        vec.clear();
    }
};

#endif // STACK_H

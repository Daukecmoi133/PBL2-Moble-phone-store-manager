#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T>
class Vector {
private:
    T* data;
    int size;
    int capacity;

    void resize(int newCapacity) {
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

public:
    Vector(int initCapacity = 8) : size(0), capacity(initCapacity > 0 ? initCapacity : 8) {
        data = new T[capacity];
    }

    // Copy Constructor
    Vector(const Vector& other) : size(other.size), capacity(other.capacity) {
        data = new T[capacity];
        for (int i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }

    // Move Constructor
    Vector(Vector&& other) noexcept : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    // Destructor
    ~Vector() {
        delete[] data;
    }

    // Copy Assignment
    Vector& operator=(const Vector& other) {
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

    // Move Assignment
    Vector& operator=(Vector&& other) noexcept {
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

    void push_back(const T& item) {
        if (size == capacity) {
            resize(capacity * 2);
        }
        data[size++] = item;
    }

    void push_back(T&& item) {
        if (size == capacity) {
            resize(capacity * 2);
        }
        data[size++] = std::move(item);
    }

    void pop_back() {
        if (size > 0) {
            --size;
        }
    }

    T& operator[](int index) {
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }

    T& at(int index) {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Vector index out of range");
        }
        return data[index];
    }

    const T& at(int index) const {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Vector index out of range");
        }
        return data[index];
    }

    bool removeAt(int index) {
        if (index < 0 || index >= size) {
            return false;
        }
        for (int i = index; i < size - 1; ++i) {
            data[i] = std::move(data[i + 1]);
        }
        --size;
        return true;
    }

    void clear() {
        size = 0;
    }

    int getSize() const { return size; }
    int getCapacity() const { return capacity; }
    bool isEmpty() const { return size == 0; }

    T* begin() { return data; }
    const T* begin() const { return data; }
    T* end() { return data + size; }
    const T* end() const { return data + size; }
};

#endif // VECTOR_H

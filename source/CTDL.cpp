// nơi nhận các cấu trúc dữ liệu cần dùng (stack, queue, list,...)
// nơi viết nghiệp vụ chính của chương trình

#include <iostream>
using namespace std;

typedef int Position;

// =============== LINKED LIST ==============
struct Node { // định nghĩa 1 node trong danh sách liên kết
    void *data;
    Node* prev;
    Node* next;

    Node(void *data) : data(data), prev(nullptr), next(nullptr) {}
    Node(void *data, Node* prev, Node* next) : data(data), prev(prev), next(next) {};

    bool hasNext() {
        return next != nullptr;
    }

    bool hasPrev() {
        return prev != nullptr;
    }

    Node *getNext() {
        return next;
    }

    Node *getPrev() {
        return prev;
    }

    void *getData() {
        return data;
    }

    bool isExist(void *data) {
        if (data == nullptr || this->data == nullptr) {
            return false;
        }
        if (data == this->data) {
            return true;
        }
        return false;
    }

    bool deleteNext(void *data) {
        if (next == nullptr) {
            return false;
        }
        Node* temp = next;
        next = next->next;
        if (next != nullptr) {
            next->prev = this;
        }
        delete temp;
        return true;
    }
};
typedef Node* LinkedList;
// =============== VECTOR ==============
struct Vector { // định nghĩa 1 vector
    void **data;
    int size;
    int capacity;

    Vector(int capacity = 10) : size(0), capacity(capacity) {
        data = new void*[capacity];
    }

    ~Vector() {
        delete[] data;
    }

    void push_back(void *item) {
        if (size == capacity) {
            capacity *= 2;
            void **newData = new void*[capacity];
            for (int i = 0; i < size; i++) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
        }
        data[size++] = item;
    }

    void pop_back() {
        if (size > 0) {
            size--;
        }
    }

    void *get(int index) const {
        if (index < 0 || index >= size) {
            return nullptr;
        }
        return data[index];
    }

    int getSize() const {
        return size;
    }
};
// ============= HASH TABLE ==============
typedef struct HashTableItemInfo {
    int id;
	void *data;
	struct HashTableItemInfo *next;
} HashTableItem;

struct HashTableInfo {
	int m;
	HashTableItem **T;
    
    HashTableInfo* create_hashtable(int m) {
        HashTableInfo* HT = (HashTableInfo*)malloc(sizeof(struct HashTableInfo));
        if (!HT) return NULL;

        HT->m = m;
        HT->T = (HashTableItem**)malloc(sizeof(HashTableItem*) * m);
        if (!HT->T) {
            free(HT);
            return NULL;
        }

        for (int i = 0; i < m; i++) 
            HT->T[i] = NULL;

        return HT;
    }

    Position Hash(int key, int m) {
        return key%m;
    }

    void insert(HashTableInfo* HT, HashTableItem* item) {
    }
};
typedef struct HashTableInfo *HashTable;
// ============== QUEUE ==============

// ============== BST ==============

// ============== HEAP ==============

#ifdef RUN_TEST_CTDL
int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    return 0;
}
#endif
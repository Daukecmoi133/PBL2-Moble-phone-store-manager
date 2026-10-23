#pragma once
// Sắp xếp tổng quát tự cài (merge sort ổn định, quick sort). Không biết thực thể nào.
// Dùng Vector tự cài; so sánh bằng functor `less(a, b)` trả true nếu a đứng trước b.

#include "../ds/Vector.h"

class Sort {
public:
    // Merge sort: ổn định, O(n log n), cần bộ nhớ phụ O(n)
    template <typename T, typename Less>
    static void mergeSort(Vector<T>& items, Less less) {
        if (items.getSize() < 2) return;
        Vector<T> buffer(items.getSize());
        for (int i = 0; i < items.getSize(); ++i) buffer.push_back(items[i]);
        mergeSortRange(items, buffer, 0, items.getSize() - 1, less);
    }

    // Quick sort: không ổn định, trung bình O(n log n), không cần bộ nhớ phụ
    template <typename T, typename Less>
    static void quickSort(Vector<T>& items, Less less) {
        if (items.getSize() < 2) return;
        quickSortRange(items, 0, items.getSize() - 1, less);
    }

private:
    template <typename T, typename Less>
    static void mergeSortRange(Vector<T>& a, Vector<T>& buf, int lo, int hi, Less less) {
        if (lo >= hi) return;
        int mid = lo + (hi - lo) / 2;
        mergeSortRange(a, buf, lo, mid, less);
        mergeSortRange(a, buf, mid + 1, hi, less);

        int i = lo, j = mid + 1, k = lo;
        while (i <= mid && j <= hi) {
            // Lấy phần tử bên phải chỉ khi nó thật sự đứng trước -> giữ tính ổn định
            if (less(a[j], a[i])) buf[k++] = a[j++];
            else buf[k++] = a[i++];
        }
        while (i <= mid) buf[k++] = a[i++];
        while (j <= hi) buf[k++] = a[j++];
        for (int t = lo; t <= hi; ++t) a[t] = buf[t];
    }

    template <typename T, typename Less>
    static void quickSortRange(Vector<T>& a, int lo, int hi, Less less) {
        while (lo < hi) {
            // Chọn pivot là phần tử giữa, phân hoạch kiểu Hoare
            T pivot = a[lo + (hi - lo) / 2];
            int i = lo, j = hi;
            while (i <= j) {
                while (less(a[i], pivot)) ++i;
                while (less(pivot, a[j])) --j;
                if (i <= j) {
                    T tmp = a[i];
                    a[i] = a[j];
                    a[j] = tmp;
                    ++i;
                    --j;
                }
            }
            // Đệ quy nhánh nhỏ, lặp nhánh lớn để giới hạn độ sâu ngăn xếp
            if (j - lo < hi - i) {
                quickSortRange(a, lo, j, less);
                lo = i;
            } else {
                quickSortRange(a, i, hi, less);
                hi = j;
            }
        }
    }
};

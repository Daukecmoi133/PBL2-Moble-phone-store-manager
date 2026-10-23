#ifndef UICOMMON_H
#define UICOMMON_H

#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>

class UICommon {
public:
    static void clearScreen();
    static void pauseScreen();
    static void printLine(int length = 80, char c = '=');
    static void printHeader(const std::string& title, int width = 80);
    static void printPhoneRow(const std::string& id, const std::string& name, 
                              const std::string& brand, const std::string& price, int qty);
    static void printPhoneTableHeader();
};

// ================= IMPLEMENTATION =================

inline void UICommon::clearScreen() {
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

inline void UICommon::pauseScreen() {
    std::cout << "\nNhấn phím Enter bất kỳ để tiếp tục...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

inline void UICommon::printLine(int length, char c) {
    std::cout << std::string(length, c) << "\n";
}

inline void UICommon::printHeader(const std::string& title, int width) {
    printLine(width, '=');
    int padding = (width - static_cast<int>(title.length())) / 2;
    if (padding < 0) padding = 0;
    std::cout << std::string(padding, ' ') << title << "\n";
    printLine(width, '=');
}

inline void UICommon::printPhoneRow(const std::string& id, const std::string& name, 
                                    const std::string& brand, const std::string& price, int qty) {
    std::cout << "| " << std::left << std::setw(6) << id
              << "| " << std::left << std::setw(25) << (name.length() > 24 ? name.substr(0, 21) + "..." : name)
              << "| " << std::left << std::setw(12) << brand
              << "| " << std::right << std::setw(15) << price
              << " | " << std::right << std::setw(6) << qty << " |\n";
}

inline void UICommon::printPhoneTableHeader() {
    printLine(74, '-');
    std::cout << "| " << std::left << std::setw(6) << "Mã SP"
              << "| " << std::left << std::setw(25) << "Tên điện thoại"
              << "| " << std::left << std::setw(12) << "Thương hiệu"
              << "| " << std::right << std::setw(15) << "Giá bán (VNĐ)"
              << " | " << std::right << std::setw(6) << "Tồn" << " |\n";
    printLine(74, '-');
}

#endif // UICOMMON_H

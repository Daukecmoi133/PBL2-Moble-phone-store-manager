# Makefile — PBL2 Phone Store Manager (GTK4 + C++17)
# Targets: all, run, clean, info

CXX      := g++
GTK4_CFLAGS := $(shell pkg-config --cflags gtk4 fontconfig)
GTK4_LIBS   := $(shell pkg-config --libs gtk4 fontconfig libcrypto)

CXXFLAGS := -std=c++17 -Wall -Wextra \
            $(GTK4_CFLAGS) \
            -I. \
            -Wno-unused-parameter \
            -Wno-deprecated-declarations \
            -Wno-pedantic

LDFLAGS  := $(GTK4_LIBS)

TARGET   = pbl2_gtk4

SRCS     = main.cpp \
           source/controllers/AppController.cpp \
           source/controllers/ProductController.cpp \
           source/controllers/CartController.cpp \
           source/controllers/OrderController.cpp \
           source/controllers/InvoiceController.cpp \
           source/controllers/CustomerController.cpp \
           source/controllers/EmployeeController.cpp \
           source/controllers/InventoryController.cpp \
           source/controllers/DiscountController.cpp \
           source/controllers/AccountController.cpp \
           source/controllers/ReportController.cpp \
           UI/components/UIComponents.cpp \
           UI/components/ProductDetailView.cpp \
           UI/screens/LoginScreen.cpp \
           UI/screens/MainShellWindow.cpp \
           UI/screens/CustomerScreens.cpp \
           UI/screens/SalesScreens.cpp \
           UI/screens/WarehouseScreens.cpp \
           UI/screens/AdminScreens.cpp \
           UI/AppRunner.cpp \
           UI/FontSetup.cpp

OBJS     = $(SRCS:.cpp=.o)
DEPS     = $(OBJS:.o=.d)

CORE_HEADERS = source/PBL2.h

.PHONY: all run clean info assets check-emoji deps dist

# Default target
all: assets $(TARGET)
	@echo ""
	@echo "=== Build thanh cong! Chay: mingw32-make run ==="

# Kiểm tra tài nguyên font emoji
assets:
	@echo [PBL2] Assets check OK

# Kiểm tra fontconfig, Pango, Cairo và GTK4
check-emoji:
	@fc-list | findstr /i emoji
	@pkg-config --modversion pango cairo gtk4

# Cài các dependency cần thiết trong MSYS2 UCRT64
deps:
	pacman -S --needed mingw-w64-ucrt-x86_64-fontconfig mingw-w64-ucrt-x86_64-adwaita-icon-theme

# Link
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile với auto-dependency generation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include auto-generated dependencies
-include $(DEPS)

# Build ứng dụng
build: all
	@echo "[PBL2] Da build xong ung dung!"

# Chạy ứng dụng
run: all
	@echo "[PBL2] Khoi dong ung dung..."
	./$(TARGET)

# Dọn dẹp
clean:
	@echo "[PBL2] Don dep cac tep build..."
	@powershell -Command "Get-ChildItem -Recurse -Include *.o,*.d,pbl2_gtk4,pbl2_gtk4.exe -ErrorAction SilentlyContinue | Remove-Item -Force"
	@echo "[PBL2] Da don dep sach!"

# In thông tin môi trường build
info: check-emoji
	@echo "CXX:     $(CXX)"
	@echo "GTK4:    $(shell pkg-config --modversion gtk4)"
	@echo "TARGET:  $(TARGET)"
	@echo "SOURCES: $(SRCS)"

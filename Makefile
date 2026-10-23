# Makefile — PBL2 Phone Store Manager (GTK4 + C++17)
# Targets: all, run, clean, info

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra \
           $(shell pkg-config --cflags gtk4) \
           -I. \
           -Isource/utils \
           -Wno-unused-parameter \
           -Wno-deprecated-declarations \
           -Wno-pedantic

LDFLAGS  = $(shell pkg-config --libs gtk4)

TARGET   = pbl2_gtk4

SRCS     = main.cpp \
           source/utils/FileStore.cpp \
           source/services/InvoiceService.cpp \
           source/controllers/AppController.cpp \
           source/ui/components/UIComponents.cpp \
           source/ui/screens/LoginScreen.cpp \
           source/ui/screens/MainShellWindow.cpp \
           source/ui/screens/CustomerScreens.cpp \
           source/ui/screens/SalesScreens.cpp \
           source/ui/screens/WarehouseScreens.cpp \
           source/ui/screens/AdminScreens.cpp \
           source/ui/AppRunner.cpp

OBJS     = $(SRCS:.cpp=.o)
DEPS     = $(OBJS:.o=.d)

CORE_HEADERS = source/PBL2.h

.PHONY: all run test clean info

# Default target
all: $(TARGET)
	@echo ""
	@echo "=== Build thanh cong! Chay: mingw32-make run ==="

# Link
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile với auto-dependency generation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include auto-generated dependencies
-include $(DEPS)

# Chạy ứng dụng
run: all
	@echo "[PBL2] Khoi dong ung dung..."
	./$(TARGET)

# Biên dịch chương trình chuyển đổi dữ liệu độc lập
MigrateData.exe: MigrateData.cpp source/utils/FileStore.cpp
	$(CXX) -std=c++17 -Wall -Wextra -I. -Isource/utils $^ -o $@

# Target kiểm thử đơn vị & tích hợp
TEST_TARGET = test_runner.exe
TEST_SRCS   = tests/unit_tests.cpp source/utils/FileStore.cpp source/services/InvoiceService.cpp

test: MigrateData.exe
	@echo "[PBL2] Bien dich bo kiem thu..."
	$(CXX) -std=c++17 -Wall -Wextra -I. -Isource/utils $(TEST_SRCS) -o $(TEST_TARGET)
	@echo "[PBL2] Khoi chay bo kiem thu..."
	./$(TEST_TARGET)
	@powershell -Command "Remove-Item $(TEST_TARGET) -ErrorAction SilentlyContinue"

# Dọn dẹp
clean:
	@echo "[PBL2] Don dep cac tep build..."
	@powershell -Command "Get-ChildItem -Recurse -Include *.o,*.d,pbl2_gtk4,pbl2_gtk4.exe,test_runner.exe -ErrorAction SilentlyContinue | Remove-Item -Force"
	@echo "[PBL2] Da don dep sach!"

# In thông tin môi trường build
info:
	@echo "CXX:     $(CXX)"
	@echo "GTK4:    $(shell pkg-config --modversion gtk4)"
	@echo "TARGET:  $(TARGET)"
	@echo "SOURCES: $(SRCS)"

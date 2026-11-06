# Makefile — PBL2 Phone Store Manager (GTK4 + C++17)
# Targets: all, run, clean, info

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra \
           $(shell pkg-config --cflags gtk4) \
           -I. \
           -Wno-unused-parameter \
           -Wno-deprecated-declarations \
           -Wno-pedantic

LDFLAGS  = $(shell pkg-config --libs gtk4)

TARGET   = pbl2_gtk4

SRCS     = main.cpp \
           source/controllers/AppController.cpp \
           UI/components/UIComponents.cpp \
           UI/screens/LoginScreen.cpp \
           UI/screens/MainShellWindow.cpp \
           UI/screens/CustomerScreens.cpp \
           UI/screens/SalesScreens.cpp \
           UI/screens/WarehouseScreens.cpp \
           UI/screens/AdminScreens.cpp \
           UI/AppRunner.cpp

OBJS     = $(SRCS:.cpp=.o)
DEPS     = $(OBJS:.o=.d)

CORE_HEADERS = source/PBL2.h

.PHONY: all run clean info

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

# Dọn dẹp
clean:
	@echo "[PBL2] Don dep cac tep build..."
	@powershell -Command "Get-ChildItem -Recurse -Include *.o,*.d,pbl2_gtk4,pbl2_gtk4.exe -ErrorAction SilentlyContinue | Remove-Item -Force"
	@echo "[PBL2] Da don dep sach!"

# In thông tin môi trường build
info:
	@echo "CXX:     $(CXX)"
	@echo "GTK4:    $(shell pkg-config --modversion gtk4)"
	@echo "TARGET:  $(TARGET)"
	@echo "SOURCES: $(SRCS)"

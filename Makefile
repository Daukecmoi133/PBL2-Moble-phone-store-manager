# Makefile — PBL2 Phone Store Manager (GTK4 + C++17)
# Targets: all, run, clean, info

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra \
           $(shell pkg-config --cflags gtk4) \
           -I. \
           -Wno-unused-parameter \
           -Wno-deprecated-declarations \
           -Wno-pedantic
CXXFLAGS += $(shell pkg-config --cflags fontconfig)

LDFLAGS  = $(shell pkg-config --libs gtk4)
LDFLAGS += $(shell pkg-config --libs fontconfig)

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
	@powershell -NoProfile -Command "if (-not (Test-Path 'UI\styles\NotoColorEmoji.ttf')) { Write-Warning 'Missing UI\styles\NotoColorEmoji.ttf; emoji font loading may fail.' }"

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

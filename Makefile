# Makefile cho dự án PBL2 (Phone Store Manager) sử dụng GTK3

# Trình biên dịch
CXX = g++

# Cờ biên dịch: sử dụng pkg-config để lấy cflags cho gtk+-3.0
CXXFLAGS = -std=c++11 -Wall `pkg-config --cflags gtk+-3.0`

# Cờ liên kết: sử dụng pkg-config để lấy libs cho gtk+-3.0
LDFLAGS = `pkg-config --libs gtk+-3.0`

# Tên file thực thi
TARGET = pbl2_app

# Nguồn file (chỉ compile main và Gtk UI, Core sử dụng headers)
SRCS = main.cpp UI/GtkAdminUI.cpp
OBJS = $(SRCS:.cpp=.o)

# Rule mặc định
all: $(TARGET)

# Build file thực thi
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Dọn dẹp
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

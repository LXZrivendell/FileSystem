CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = filesystem
SRCDIR = 文件系统
SOURCES = $(SRCDIR)/filesystem.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
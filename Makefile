CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = filesystem
SRCDIR = core
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
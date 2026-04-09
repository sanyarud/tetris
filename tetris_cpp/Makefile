CXX = clang++
CXXFLAGS = -std=c++17 -O2 -Wall -I/opt/homebrew/include/SDL2 -F/opt/homebrew/Frameworks
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf

TARGET = tetris
SRC = main.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
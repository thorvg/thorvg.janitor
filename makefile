TARGET = tvggame
SRC = tvggame.cpp

all:
	g++ $(SRC) -o $(TARGET) -O3 -std=c++20 $(shell sdl2-config --cflags --libs) -lthorvg -I/opt/homebrew/include

clean:
	rm -f $(TARGET)

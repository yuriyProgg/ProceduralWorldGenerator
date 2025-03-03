TARGET=ProceduralWorldGenerator
CC=g++
FLAGS=-w -O3 -std=c++23
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(FLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TARGET)

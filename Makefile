CXX = g++
CXXFLAGS = -I./include -std=c++17
LDFLAGS = -lgmp

SOURCES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))

program: src/main.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run:
	./program

clean:
	rm -f program
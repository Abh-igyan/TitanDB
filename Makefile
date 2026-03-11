CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

SRC      = src/titandb.cpp
MAIN     = src/main.cpp
TEST     = tests/test_titandb.cpp

.PHONY: all run test clean

all: titandb

titandb: $(SRC) $(MAIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(SRC) $(TEST)
	$(CXX) $(CXXFLAGS) $^ -o test_runner
	./test_runner

run: titandb
	./titandb

clean:
	rm -f titandb test_runner *.db

# build:
# 	g++ -std=c++17 -o spl_compiler main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp

# run:
# 	./spl_compiler tests/valid/valid_program.txt

# test:
# 	./run_organized_tests.sh
# 	g++ -std=c++17 tests/ICG/ICG_test.cpp -o test
# 	./test

# clean:
# 	rm -f spl_compiler *.o ICG.txt

.PHONY: build run test test_organized clean

CXX = g++
CXXFLAGS = -std=c++17
LDFLAGS = -lfl

# ------------------- Source files -------------------

# Main program sources
SOURCES = main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp
OBJ = $(SOURCES:.cpp=.o)

# Test sources (exclude main.cpp, include test main)
TEST_SOURCES = tests/ICG/ICG_test.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp
TEST_OBJ = $(TEST_SOURCES:.cpp=.o)

# ------------------- Build targets -------------------

build: $(OBJ)
	$(CXX) $(CXXFLAGS) -o spl_compiler $(OBJ) $(LDFLAGS)

test: clean $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o test $(TEST_OBJ) $(LDFLAGS)
	./test

test_organized:
	./run_organized_tests.sh

run: build
	./spl_compiler tests/valid/valid_program.txt

# ------------------- Compilation rule -------------------

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------- Clean -------------------

clean:
	rm -f $(OBJ) $(TEST_OBJ) spl_compiler test ICG.txt

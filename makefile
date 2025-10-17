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

SOURCES = main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp
OBJ = $(SOURCES:.cpp=.o)

TEST_SOURCES = tests/ICG/ICG_test.cpp
TEST_OBJ = ICG_test.o spl.tab.o spl_lexer.o lexer_bridge.o type_checker.o codegen.o

# ------------------- Build -------------------

build: $(OBJ)
	$(CXX) $(CXXFLAGS) -o spl_compiler $(OBJ) -lfl

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------- Run -------------------

run:
	./spl_compiler tests/valid/valid_program.txt

# ------------------- Tests -------------------

test_organized:
	./run_organized_tests.sh

test: $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o test $(TEST_OBJ) -lfl
	./test

ICG_test.o: tests/ICG/ICG_test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

spl.tab.o: spl.tab.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

spl_lexer.o: spl_lexer.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

lexer_bridge.o: lexer_bridge.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

type_checker.o: type_checker.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

codegen.o: Intermediate-Code-Generation/codegen.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------- Clean -------------------

clean:
	rm -f *.o spl_compiler test ICG.txt

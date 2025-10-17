.PHONY: build run test clean

build:
	g++ -std=c++17 -o spl_compiler main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp

run:
	./spl_compiler tests/valid/valid_program.txt

test:
	./run_organized_tests.sh
	g++ -std=c++17 tests/ICG/ICG_test.cpp -o test
	./test

clean:
	rm -f spl_compiler *.o ICG.txt test

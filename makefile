.PHONY: build run test test_organized clean submission

CXX = g++
CXXFLAGS = -std=c++17
LDFLAGS = -lfl

# --- Static Flags for Submission Build ---
# This tells g++ to bundle all libraries (like libfl, libstdc++)
# into the executable, so it runs on any machine.
LDFLAGS_STATIC = -lfl -static

# ------------------- Source files -------------------
# FIXED: Added the correct path to codegen.cpp
SOURCES = main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp
OBJ = $(SOURCES:.cpp=.o)

# FIXED: Added the correct path to codegen.cpp
TEST_SOURCES = tests/ICG/ICG_test.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp
TEST_OBJ = $(TEST_SOURCES:.cpp=.o)

# ------------------- Main Targets -------------------

# Standard build (dynamic, for local testing)
build: $(OBJ)
	$(CXX) $(CXXFLAGS) -o spl_compiler $(OBJ) $(LDFLAGS)

# SUBMISSION TARGET (static, for final submission)
submission: $(OBJ)
	@echo "--- Building STATIC executable for submission ---"
	$(CXX) $(CXXFLAGS) -o spl_compiler $(OBJ) $(LDFLAGS_STATIC)
	@echo "--- Creating submission.zip (Requires user_manual.pdf) ---"
	@zip submission.zip spl_compiler user_manual.pdf
	@echo "--- Done! Upload submission.zip ---"

run: build
	./spl_compiler tests/valid/valid_program.txt

test: clean $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o test $(TEST_OBJ) $(LDFLAGS)
	./test

test_organized:
	./run_organized_tests.sh

# ------------------- Compilation rule -------------------
# This rule handles compiling .cpp files from the root directory
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# This rule handles compiling .cpp files from the subdirectory
Intermediate-Code-Generation/%.o: Intermediate-Code-Generation/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------- Clean -------------------
clean:
	rm -f $(OBJ) $(TEST_OBJ) spl_compiler test ICG.txt ICG.html submission.zip

# ------------------- End of Makefile -------------------
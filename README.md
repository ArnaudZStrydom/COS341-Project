# COS341-Project
COS341 Project (SPL Compiler with Type Checker and Intermediate Code generation)

## Overview
This project implements a complete compiler for SPL (Students' Programming Language) including:
- **Lexer**: Tokenizes SPL source code
- **Parser**: Builds an Abstract Syntax Tree (AST)
- **Type Checker**: Validates type correctness according to SPL specification
- **Intermediate Code Generation**: Generates intermediate code

## Build Instructions

### Build Parser
```bash
bison -d spl.y
```

### Build Complete Compiler
```bash
g++ -std=c++17 -o spl_compiler main.cpp spl.tab.cpp spl_lexer.cpp lexer_bridge.cpp type_checker.cpp Intermediate-Code-Generation/codegen.cpp
```
#### or 
```bash
make build
make run
make clean
```
## Usage

### Compile and Type Check
```bash
./spl_compiler tests/valid/valid_program.txt
```

### Run Test Suite
```bash
# Run all organized tests
./run_organized_tests.sh

# Run simple tests only
for test in tests/simple/*.txt; do ./spl_compiler "$test"; done

# Run type checker tests only  
for test in tests/type_checker/*.txt; do ./spl_compiler "$test"; done
```
### Run Doctests
```bash
make test
```



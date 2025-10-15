#!/bin/bash

# SPL Type Checker Organized Test Suite
# Tests organized into simple and type_checker folders

echo "=========================================="
echo "  SPL TYPE CHECKER ORGANIZED TESTS"
echo "=========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

# Function to run a test
run_test() {
    local test_file="$1"
    local expected_result="$2"  # "pass" or "fail"
    local description="$3"
    
    echo -e "${BLUE}Testing: $test_file${NC}"
    echo "Description: $description"
    echo "Expected: $expected_result"
    
    if [ "$expected_result" = "pass" ]; then
        if ./spl_compiler "$test_file" 2>&1 | grep -q "Type checking passed"; then
            echo -e "${GREEN}✓ PASSED${NC} - Valid program accepted"
            ((PASSED++))
        else
            echo -e "${RED}✗ FAILED${NC} - Valid program rejected"
            ((FAILED++))
        fi
    else
        if ./spl_compiler "$test_file" 2>&1 | grep -q "Type checking failed"; then
            echo -e "${GREEN}✓ PASSED${NC} - Type error correctly detected"
            ((PASSED++))
        else
            echo -e "${RED}✗ FAILED${NC} - Type error not detected"
            ((FAILED++))
        fi
    fi
    echo ""
}

echo "=== SIMPLE TESTS (tests/simple/) ==="
echo "Basic valid programs that should pass type checking:"
echo ""

# Simple tests - all should pass
run_test "tests/simple/test_valid_simple.txt" "pass" "Simple valid program with arithmetic"
run_test "tests/simple/test_complex_valid.txt" "pass" "Complex valid program with functions and procedures"
run_test "tests/valid/valid_program.txt" "pass" "Original comprehensive valid program"

echo "=== TYPE CHECKER TESTS (tests/type_checker/) ==="
echo "Tests that verify type checking functionality:"
echo ""

# Type error tests - should fail
run_test "tests/type_checker/test_type_error_boolean_to_numeric.txt" "fail" "Boolean expression assigned to numeric variable"
run_test "tests/type_checker/test_undeclared_variable.txt" "fail" "Undeclared variable used in assignment"
run_test "tests/type_checker/test_undeclared_function.txt" "fail" "Undeclared function called"
run_test "tests/type_checker/test_undeclared_procedure.txt" "fail" "Undeclared procedure called"

# Valid operator tests - should pass
run_test "tests/type_checker/test_arithmetic_operators.txt" "pass" "Valid arithmetic operators (plus, minus, mult, div)"
run_test "tests/type_checker/test_comparison_operators.txt" "pass" "Valid comparison operators (>, eq)"
run_test "tests/type_checker/test_boolean_operators.txt" "pass" "Valid boolean operators (and, or)"
run_test "tests/type_checker/test_unary_operators.txt" "pass" "Valid unary operators (neg)"

# Type error operator tests - should fail
run_test "tests/type_checker/test_arithmetic_operator_type_error.txt" "fail" "Boolean expression in arithmetic operator"
run_test "tests/type_checker/test_comparison_operator_type_error.txt" "fail" "Boolean expression in comparison operator"
run_test "tests/type_checker/test_boolean_operator_type_error.txt" "fail" "Mixed boolean and numeric in boolean operator"
run_test "tests/type_checker/test_unary_operator_type_error.txt" "fail" "Invalid unary operator (not applied to numeric)"

# Function and procedure tests
run_test "tests/type_checker/test_function_definition.txt" "pass" "Valid function definition and call"
run_test "tests/type_checker/test_procedure_definition.txt" "pass" "Valid procedure definition and call"

# Scope management tests
run_test "tests/type_checker/test_scope_management.txt" "pass" "Valid scope management with global, local, and parameters"
run_test "tests/type_checker/test_scope_error.txt" "fail" "Undeclared variable in procedure scope"

# Control flow tests
run_test "tests/type_checker/test_while_loop.txt" "pass" "Valid while loop with boolean condition"
run_test "tests/type_checker/test_do_until_loop.txt" "pass" "Valid do-until loop with boolean condition"
run_test "tests/type_checker/test_if_else.txt" "pass" "Valid if-else statement with boolean condition"

# Print statement tests
run_test "tests/type_checker/test_print_statements.txt" "pass" "Valid print statements with numeric and string"

echo "=========================================="
echo "  TEST SUMMARY"
echo "=========================================="
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo "Total:  $((PASSED + FAILED))"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 All tests passed!${NC}"
    echo -e "${GREEN}Type checker is working perfectly!${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠️  Some tests failed.${NC}"
    echo -e "${YELLOW}This may be due to syntax issues or type checker bugs.${NC}"
    exit 1
fi

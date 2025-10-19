#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../../Intermediate-Code-Generation/codegen.h"
#include "../../type_checker.h"
#include "../../ast.h"
#include "../../spl.tab.hpp"
//#include "../../lexer_bridge.cpp"
#include <fstream>
#include <sstream>

extern AstNode* ast_root;
extern void initialize_lexer(const std::string& source);
extern int yyparse();


std::string readFileToString(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    return buffer.str();
}

TEST_CASE("Test most simple") {
    std::string src = readFileToString("tests/ICG/testfiles/simple.txt");

    initialize_lexer(src);
    int res = yyparse();
    if(ast_root){
        TypeChecker typeChecker;
        bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
        CodeGen codeGen;
        codeGen.setSymbolTable(&typeChecker.getSymbolTable());
        codeGen.generate(static_cast<ProgramNode*>(ast_root));
        //codeGen.printCode();
        std::vector<std::string> memo;
        memo.push_back("STOP");

        CHECK(codeGen.code == memo);
        memo.clear();
    }

    delete ast_root;
    ast_root = NULL;
}

TEST_CASE("Test simple if") {
    std::string src = readFileToString("tests/ICG/testfiles/simple_if.txt");

    initialize_lexer(src);
    int res = yyparse();
    
    REQUIRE(ast_root != nullptr);  

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));
    //codeGen.printCode();

    CHECK(codeGen.toString() == "a = 0IF 1 = 1 THEN LBL_THEN_1GOTO LBL_EXIT_2REM LBL_THEN_1STOPREM LBL_EXIT_2");

    delete ast_root;
    ast_root = NULL;
}

TEST_CASE("Test simple function") {
    std::string src = readFileToString("tests/ICG/testfiles/simple_function.txt");

    initialize_lexer(src);
    int res = yyparse();
    
    REQUIRE(ast_root != nullptr);  

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));
    //codeGen.printCode();
    //std::cout<<codeGen.toString()<<std::endl;

    CHECK(codeGen.toString() == "FUNCTION identity()temp = xRETURN tempEND FUNCTIONSTOP");

    delete ast_root;
    ast_root = NULL;
}

TEST_CASE("Test simple if-else") {
    std::string src = readFileToString("tests/ICG/testfiles/simple_if_else.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    CHECK(codeGen.toString() ==
        "IF 1 > 100 THEN LBL_THEN_1"
        "GOTO LBL_ELSE_2"
        "REM LBL_ELSE_2"
        "PRINT \"gcountnotlarge\""
        "GOTO LBL_EXIT_3"
        "REM LBL_THEN_1"
        "PRINT \"gcounterislarge\""
        "REM LBL_EXIT_3"
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}

TEST_CASE("Test simple while loop") {
    std::string src = readFileToString("tests/ICG/testfiles/simple_while.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    //codeGen.printCode();

    CHECK(codeGen.toString() ==
        "REM LBL_WHILE_1"
        "IF 100 > x THEN LBL_WHILE_1_BODY"
        "GOTO LBL_EXIT_WHILE_2"
        "REM LBL_WHILE_1_BODY"
        "t1 = x"
        "t2 = 1"
        "t3 = t1 + t2"
        "x = t3"
        "GOTO LBL_WHILE_1"
        "REM LBL_EXIT_WHILE_2"
        "PRINT \"Heybrother\""
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}

TEST_CASE("test_unary_operators.txt") {
    std::string src = readFileToString("tests/type_checker/test_unary_operators.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    //codeGen.printCode();

    CHECK(codeGen.toString() ==
        "t1 = -5"
        "x = t1"
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}

TEST_CASE("test_function_definition.txt") {
    std::string src = readFileToString("tests/type_checker/test_function_definition.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    //codeGen.printCode();

    CHECK(codeGen.toString() ==
        "FUNCTION add()"
        "RETURN a"
        "END FUNCTION"
        ""
        "t1 = CALL_add(5)"
        "x = t1"
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}

TEST_CASE("test_procedure_definition.txt") {
    std::string src = readFileToString("tests/type_checker/test_procedure_definition.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    //codeGen.printCode();

    CHECK(codeGen.toString() ==
        "PROC printnum()"
        "PRINT n"
        "END PROC"
        ""
        "x = 5"
        "CALL_printnum(x)"
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}

TEST_CASE("test_scope_management.txt") {
    std::string src = readFileToString("tests/type_checker/test_scope_management.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    //codeGen.printCode();

    CHECK(codeGen.toString() ==
        "PROC testproc()"
        "globalvar = 10"
        "param1 = 5"
        "localvar = 3"
        "STOP"
        "END PROC"
        ""
        "FUNCTION testfunc()"
        "t1 = param1"
        "t2 = param2"
        "t3 = t1 + t2"
        "localvar = t3"
        "RETURN localvar"
        "END FUNCTION"
        ""
        "mainvar = 1"
        "CALL_testproc(mainvar)"
        "t4 = CALL_testfunc(2,3)"
        "mainvar = t4"
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}

TEST_CASE("test_do_until_loop.txt") {
    std::string src = readFileToString("tests/type_checker/test_do_until_loop.txt");

    initialize_lexer(src);
    int res = yyparse();
    REQUIRE(ast_root != nullptr);

    TypeChecker typeChecker;
    bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
    REQUIRE(typeCheckPassed);

    CodeGen codeGen;
    codeGen.setSymbolTable(&typeChecker.getSymbolTable());
    codeGen.generate(static_cast<ProgramNode*>(ast_root));

    //codeGen.printCode();

    CHECK(codeGen.toString() ==
        "i = 0"
        "REM LBL_DO_1"
        "t1 = i"
        "t2 = 1"
        "t3 = t1 + t2"
        "i = t3"
        "IF i > 5 THEN LBL_EXIT_DO_2"
        "GOTO LBL_DO_1"
        "REM LBL_EXIT_DO_2"
        "STOP"
    );

    delete ast_root;
    ast_root = nullptr;
}
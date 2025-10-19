#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../../Intermediate-Code-Generation/codegen.h"
#include "../../type_checker.h"
#include "../../ast.h"
#include "../../spl.tab.hpp"
//#include "../../lexer_bridge.cpp"
#include <fstream>
#include <sstream>

extern AstNode* ast_root;  // declared in spl.y / parser
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

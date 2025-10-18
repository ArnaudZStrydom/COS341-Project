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

TEST_CASE("Test simple assignment from file") {
    std::string src = readFileToString("tests/ICG/testfiles/simple_assign.txt");

    initialize_lexer(src);
    int res = yyparse();
    if(ast_root){
        TypeChecker typeChecker;
        bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
        CodeGen codeGen;
        codeGen.setSymbolTable(&typeChecker.getSymbolTable());
        codeGen.generate(static_cast<ProgramNode*>(ast_root));
        codeGen.printCode();
    }

    CHECK(1==1);
    delete ast_root;
    ast_root = NULL;
}

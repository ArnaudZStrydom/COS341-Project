#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../../Intermediate-Code-Generation/codegen.h"
#include "../../type_checker.h"
#include "../../ast.h"
#include "../../lexer_bridge.cpp"
#include <fstream>
#include <sstream>
extern AstNode* ast_root;  // declared in spl.y / parser
extern void initialize_lexer(const std::string& source);
extern int yyparse();


std::string readFile(const std::string& path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

TEST_CASE("Test simple assignment from file") {
    std::string src = readFile("testfiles/simple_assign.txt");

    // initialize lexer and parse
    initialize_lexer(src);
    yyparse();

    REQUIRE(ast_root != nullptr);

    TypeChecker tc;
    REQUIRE(tc.typeCheck(static_cast<ProgramNode*>(ast_root)));

    CodeGen cg;
    cg.setSymbolTable(&tc.getSymbolTable());
    cg.generate(static_cast<ProgramNode*>(ast_root));

    // We can assert some code exists
    bool found = false;
    for (auto& line : cg.code) {
        if (line.find("t1 = 5 + 3") != std::string::npos) found = true;
    }
    CHECK(found);

    delete ast_root;
    ast_root = nullptr;
}

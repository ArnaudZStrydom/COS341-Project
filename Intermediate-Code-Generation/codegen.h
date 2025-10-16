#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <vector>
#include "../ast.h"

class CodeGen {
public:
    std::vector<std::string> code;

    // Main method to generate code from the AST
    void generate(ProgramNode* program);

    // Save the generated code to a file
    void saveCode() const;

private:
    // Temp counter for generating temporaries like t1, t2, ...
    int tempCounter = 0;
    std::string newTemp();

    // Helpers for generating code
    std::string genProgram(ProgramNode* program);
    void genStatementList(AstNodeList<StatementNode>* stmts);
    void genStatement(StatementNode* stmt);
    std::string genExpression(ExpressionNode* expr);

    // Utility to append a line to the output
    void emit(const std::string& line);
};

#endif // CODEGEN_H

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
    // Helpers for generating code
    std::string genProgram(ProgramNode* program);
    std::string genStatementList(AstNodeList<StatementNode>* stmts);
    std::string genStatement(StatementNode* stmt);
    std::string genExpression(ExpressionNode* expr);
};

#endif // CODEGEN_H

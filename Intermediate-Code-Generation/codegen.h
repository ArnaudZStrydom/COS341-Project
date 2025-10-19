#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <vector>
#include "../ast.h"
#include "../type_checker.h"  // include for SymbolTable and Type

class CodeGen {
public:
    std::vector<std::string> code;

    // Allow optional symbol table injection
    CodeGen(const SymbolTable* symtab = nullptr) : symbolTable(symtab) {}

    void generate(ProgramNode* program);
    void saveCode() const;
    void printCode() const;
    std::string toString() const;

    // Optional: attach symbol table later
    void setSymbolTable(const SymbolTable* symtab) { symbolTable = symtab; }

private:
    int tempCounter = 0;
    int labelCounter = 0;  // For generating unique labels
    const SymbolTable* symbolTable; // read-only pointer to type checker’s table

    std::string newTemp();
    std::string newLabel(const std::string& prefix);
    void emit(const std::string& line);

    std::string genProgram(ProgramNode* program);
    void genStatementList(AstNodeList<StatementNode>* stmts);
    void genStatement(StatementNode* stmt);

    // `inCondition` = true avoids generating temporaries for comparisons
    std::string genExpression(ExpressionNode* expr, bool inCondition = false);

    // Conditional generation that flattens boolean expressions
    void genCondition(ExpressionNode* expr, const std::string& labelTrue, const std::string& labelFalse);

    std::string resolveVariable(const std::string& name) const;
};

#endif // CODEGEN_H

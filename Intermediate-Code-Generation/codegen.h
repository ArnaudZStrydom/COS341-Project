#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <vector>
#include "../ast.h"
#include "../type_checker.h"
#include <map>

class CodeGen {
public:
    std::vector<std::string> code;
    std::map<std::string, int> lineLabelMap;

    CodeGen(const SymbolTable* symtab = nullptr) : symbolTable(symtab) {}

    void generate(ProgramNode* program);
    void saveCode() const;
    void printCode() const;
    std::string toString() const;

    void setSymbolTable(const SymbolTable* symtab) { symbolTable = symtab; }
    void saveToHTML() const;
    void startPostProcess();


private:
    int tempCounter = 0;
    int labelCounter = 0;  
    const SymbolTable* symbolTable; 

    std::string newTemp();
    std::string newLabel(const std::string& prefix);
    void emit(const std::string& line);

    std::string genProgram(ProgramNode* program);
    void genStatementList(AstNodeList<StatementNode>* stmts);
    void genStatement(StatementNode* stmt);

    std::string genExpression(ExpressionNode* expr, bool inCondition = false);

    void genCondition(ExpressionNode* expr, const std::string& labelTrue, const std::string& labelFalse);

    std::string resolveVariable(const std::string& name) const;

    // Gather appropriate labels
    void gatherLabel(const std::string line);
    // Where appropriate change labels to line numbers
    void changeLabelToLineNumber(std::string &line);
};

#endif // CODEGEN_H

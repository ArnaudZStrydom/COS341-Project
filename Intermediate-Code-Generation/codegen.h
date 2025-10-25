#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <vector>
#include <map>
#include "../ast.h"
#include "../type_checker.h"

// Define a type for the variable rename map
typedef std::map<std::string, std::string> VarRenameMap;

class CodeGen {
public:
    std::vector<std::string> code;
    std::map<std::string, int> lineLabelMap;

    CodeGen(const SymbolTable* symtab = nullptr) : symbolTable(symtab) {}

    // --- Main Public API ---
    void generate(ProgramNode* program);
    void performInlining();
    void startPostProcess();
    
    void saveCode() const;
    void printCode() const;
    std::string toString() const;

    void setSymbolTable(const SymbolTable* symtab) { symbolTable = symtab; }
    void saveToHTML() const;


private:
    int tempCounter = 0;
    int labelCounter = 0;  
    int inlineCounter = 0; // For unique variable renaming during inlining
    const SymbolTable* symbolTable; 
    ProgramNode* astProgramRoot = nullptr; // Store root for lookups

    // --- Inlining Helpers ---
    std::string newInlinedVar(const std::string& varName);

    // --- Generation Helpers ---
    std::string newTemp();
    std::string newLabel(const std::string& prefix);
    void emit(const std::string& line);
    void emit(const std::string& line, std::vector<std::string>& codeBlock);

    std::string genProgram(ProgramNode* program);
    
    // Functions now take a varMap to handle renamed variables
    void genStatementList(AstNodeList<StatementNode>* stmts, std::vector<std::string>& codeBlock, VarRenameMap& varMap, const std::string& funcReturnVar = "");
    void genStatement(StatementNode* stmt, std::vector<std::string>& codeBlock, VarRenameMap& varMap, const std::string& funcReturnVar = "");

    std::string genExpression(ExpressionNode* expr, std::vector<std::string>& codeBlock, VarRenameMap& varMap, bool inCondition = false);
    std::string genAtom(ExpressionNode* atom, VarRenameMap& varMap);

    void genCondition(ExpressionNode* expr, std::vector<std::string>& codeBlock, VarRenameMap& varMap, const std::string& labelTrue, const std::string& labelFalse);

    std::string resolveVariable(const std::string& name, VarRenameMap& varMap);

    // --- Post-Processing Helpers ---
    void gatherLabel(const std::string line);
    void changeLabelToLineNumber(std::string &line);
};

#endif // CODEGEN_H
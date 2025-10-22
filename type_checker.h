#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "ast.h"

// Type definitions
enum class Type {
    NUMERIC,
    BOOLEAN,
    COMPARISON,
    TYPELESS,  // For function/procedure names
    UNKNOWN    // For error cases
};

// Helper function to convert Type enum to string
std::string typeToString(Type type);

// Symbol Table for tracking variable and function types
class SymbolTable {
private:
    std::unordered_map<std::string, Type> symbols;
    std::vector<std::unordered_map<std::string, Type>> scopes; // For nested scopes
    
public:
    SymbolTable();
    ~SymbolTable();
    
    // Scope management
    void enterScope();
    void exitScope();
    
    // Symbol management
    bool declare(const std::string& name, Type type);
    bool isDeclared(const std::string& name) const;
    Type getType(const std::string& name) const;
    bool isTypeLess(const std::string& name) const;
    
    // Debugging
    void printSymbols() const;
    //getter for symbols
    std::unordered_map<std::string, Type> getSymbols() const;
    //getter for scopes
    const std::vector<std::unordered_map<std::string, Type>>& getScopes() const;

};

// Type Checker class
class TypeChecker {
private:
    SymbolTable symbolTable;
    bool hasErrors;
    std::vector<std::string> errorMessages;
    
    // Helper methods
    void addError(const std::string& message);
    bool isCorrectlyTyped() const;
    
    // Type checking methods for each AST node type
    Type checkExpression(ExpressionNode* expr);
    Type checkStatement(StatementNode* stmt);
    Type checkAtom(ExpressionNode* atom);
    Type checkTerm(ExpressionNode* term);
    Type checkUnaryOp(UnaryOpNode* unaryOp);
    Type checkBinaryOp(BinaryOpNode* binaryOp);
    Type checkFuncCall(FuncCallNode* funcCall);
    Type checkVar(VarNode* var);
    
    // Statement checking methods
    bool checkAssign(AssignNode* assign);
    bool checkPrint(PrintNode* print);
    bool checkProcCall(ProcCallNode* procCall);
    bool checkIf(IfNode* ifNode);
    bool checkIfElse(IfElseNode* ifElseNode);
    bool checkWhile(WhileNode* whileNode);
    bool checkDoUntil(DoUntilNode* doUntilNode);
    bool checkHalt(HaltNode* halt);
    bool checkReturn(ReturnNode* returnNode);
    
    // List checking methods
    bool checkStatementList(AstNodeList<StatementNode>* statements);
    bool checkExpressionList(AstNodeList<ExpressionNode>* expressions);
    bool checkVarList(AstNodeList<VarNode>* variables);
    
    // Definition checking methods
    bool checkBody(BodyNode* body);
    bool checkProcDef(ProcDefNode* procDef);
    bool checkFuncDef(FuncDefNode* funcDef);
    bool checkMainProg(MainProgNode* mainProg);
    bool checkProgram(ProgramNode* program);
    
    // Parameter and input checking
    bool checkParams(AstNodeList<VarNode>* params);
    bool checkInput(AstNodeList<ExpressionNode>* input);
    bool checkOutput(ExpressionNode* output);
    
public:
    TypeChecker();
    ~TypeChecker();
    
    // Main type checking method
    bool typeCheck(ProgramNode* program);
    
    // Error reporting
    bool hasTypeErrors() const;
    const std::vector<std::string>& getErrorMessages() const;
    void printErrors() const;
    
    // Debugging
    void printSymbolTable() const;
    //symbol table getter
    const SymbolTable& getSymbolTable() const { return symbolTable; }
};

#endif // TYPE_CHECKER_H


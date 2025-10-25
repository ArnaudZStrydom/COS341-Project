#include "type_checker.h"
#include <iostream>
#include <sstream>

// Helper function to convert Type enum to string
std::string typeToString(Type type) {
    switch (type) {
        case Type::NUMERIC: return "numeric";
        case Type::BOOLEAN: return "boolean";
        case Type::COMPARISON: return "comparison";
        case Type::TYPELESS: return "typeless";
        case Type::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

// SymbolTable implementation
SymbolTable::SymbolTable() {
    enterScope(); // Start with global scope
}

SymbolTable::~SymbolTable() {
    // Clean up scopes
    while (!scopes.empty()) {
        exitScope();
    }
}

void SymbolTable::enterScope() {
    scopes.push_back(std::unordered_map<std::string, Type>());
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

bool SymbolTable::declare(const std::string& name, Type type) {
    if (scopes.empty()) {
        return false;
    }
    
    // Check if already declared in current scope
    if (scopes.back().find(name) != scopes.back().end()) {
        return false;
    }
    
    scopes.back()[name] = type;
    return true;
}

bool SymbolTable::isDeclared(const std::string& name) const {
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

Type SymbolTable::getType(const std::string& name) const {
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return Type::UNKNOWN;
}

bool SymbolTable::isTypeLess(const std::string& name) const {
    return !isDeclared(name);
}

void SymbolTable::printSymbols() const {
    std::cout << "=== Symbol Table ===" << std::endl;
    for (size_t i = 0; i < scopes.size(); ++i) {
        std::cout << "Scope " << i << ":" << std::endl;
        for (const auto& pair : scopes[i]) {
            std::cout << "  " << pair.first << " : " << typeToString(pair.second) << std::endl;
        }
    }
    std::cout << "===================" << std::endl;
}

// TypeChecker implementation
TypeChecker::TypeChecker() : hasErrors(false) {}

TypeChecker::~TypeChecker() {}

void TypeChecker::addError(const std::string& message) {
    hasErrors = true;
    errorMessages.push_back(message);
}

bool TypeChecker::isCorrectlyTyped() const {
    return !hasErrors;
}

bool TypeChecker::typeCheck(ProgramNode* program) {
    if (!program) {
        addError("Program node is null");
        return false;
    }
    
    // Reset state
    hasErrors = false;
    errorMessages.clear();
    
    // Type check the program according to SPL_PROG rule
    // SPL_PROG is correctly typed if VARIABLES, PROCDEFS, FUNCDEFS, and MAINPROG are correctly typed
    bool result = checkProgram(program);
    
    return result && isCorrectlyTyped();
}

bool TypeChecker::checkProgram(ProgramNode* program) {
    // Check global variables first
    if (program->globals && !checkVarList(program->globals)) {
        return false;
    }
    
    // First pass: declare all procedures and functions
    if (program->procs) {
        for (auto* procDef : program->procs->elements) {
            // Just declare the procedure name, don't check body yet
            if (!symbolTable.isTypeLess(procDef->name)) {
                addError("Procedure " + procDef->name + " is already declared");
                return false;
            }
            symbolTable.declare(procDef->name, Type::TYPELESS);
        }
    }
    
    if (program->funcs) {
        for (auto* funcDef : program->funcs->elements) {
            // Just declare the function name, don't check body yet
            if (!symbolTable.isTypeLess(funcDef->name)) {
                addError("Function " + funcDef->name + " is already declared");
                return false;
            }
            symbolTable.declare(funcDef->name, Type::TYPELESS);
        }
    }
    
    // Second pass: check procedure definitions
    if (program->procs) {
        for (auto* procDef : program->procs->elements) {
            if (!checkProcDef(procDef)) {
                return false;
            }
        }
    }
    
    // Second pass: check function definitions
    if (program->funcs) {
        for (auto* funcDef : program->funcs->elements) {
            if (!checkFuncDef(funcDef)) {
                return false;
            }
        }
    }
    
    // Check main program
    if (program->main && !checkMainProg(program->main)) {
        return false;
    }
    
    return true;
}

bool TypeChecker::checkVarList(AstNodeList<VarNode>* variables) {
    if (!variables) {
        return true; // Empty list is correctly typed (fact)
    }
    
    // VARIABLES is correctly typed if each VAR is of type "numeric" and remaining VARIABLES is correctly typed
    for (auto* var : variables->elements) {
        Type varType = checkVar(var);
        if (varType != Type::NUMERIC) {
            addError("Variable " + var->name + " is not correctly typed");
            return false;
        }
        
        // Declare variable as numeric type
        if (!symbolTable.declare(var->name, Type::NUMERIC)) {
            addError("Variable " + var->name + " is already declared");
            return false;
        }
    }
    
    return true;
}

Type TypeChecker::checkVar(VarNode* var) {
    if (!var) {
        addError("Var node is null");
        return Type::UNKNOWN;
    }
    
    // VAR is of type "numeric" (fact)
    return Type::NUMERIC;
}

bool TypeChecker::checkProcDef(ProcDefNode* procDef) {
    if (!procDef) {
        addError("ProcDef node is null");
        return false;
    }
    
    // PDEF is correctly typed if NAME is type-less, PARAM is correctly typed, and BODY is correctly typed
    
    // Enter new scope for procedure
    symbolTable.enterScope();
    
    // Check parameters
    if (!checkParams(procDef->params)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Check body
    if (!checkBody(procDef->body)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Exit procedure scope
    symbolTable.exitScope();
    
    return true;
}

bool TypeChecker::checkFuncDef(FuncDefNode* funcDef) {
    if (!funcDef) {
        addError("FuncDef node is null");
        return false;
    }
    
    // FDEF is correctly typed if NAME is type-less, PARAM is correctly typed, BODY is correctly typed, and ATOM is of type "numeric"
    
    // Enter new scope for function
    symbolTable.enterScope();
    
    // Check parameters
    if (!checkParams(funcDef->params)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Check body
    if (!checkBody(funcDef->body)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Check for return statement with numeric type
    // This is handled in the body checking, but we need to ensure there's a return statement
    // For now, we'll assume the body checking handles this
    
    // Exit function scope
    symbolTable.exitScope();
    
    return true;
}

bool TypeChecker::checkParams(AstNodeList<VarNode>* params) {
    if (!params) {
        return true; // Empty params is correctly typed (fact)
    }
    
    // PARAM is correctly typed if MAXTHREE is correctly typed
    // MAXTHREE is correctly typed if each VAR is of type "numeric"
    for (auto* param : params->elements) {
        Type paramType = checkVar(param);
        if (paramType != Type::NUMERIC) {
            addError("Parameter " + param->name + " is not correctly typed");
            return false;
        }
        
        // Declare parameter as numeric type
        if (!symbolTable.declare(param->name, Type::NUMERIC)) {
            addError("Parameter " + param->name + " is already declared");
            return false;
        }
    }
    
    return true;
}

bool TypeChecker::checkBody(BodyNode* body) {
    if (!body) {
        addError("Body node is null");
        return false;
    }
    
    // BODY is correctly typed if MAXTHREE is correctly typed and ALGO is correctly typed
    
    // Enter new scope for local variables
    symbolTable.enterScope();
    
    // Check local variables (MAXTHREE)
    if (!checkVarList(body->locals)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Check algorithm (ALGO)
    if (!checkStatementList(body->statements)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Exit local scope
    symbolTable.exitScope();
    
    return true;
}

bool TypeChecker::checkMainProg(MainProgNode* mainProg) {
    if (!mainProg) {
        addError("MainProg node is null");
        return false;
    }
    
    // MAINPROG is correctly typed if VARIABLES is correctly typed and ALGO is correctly typed
    
    // Enter new scope for main program
    symbolTable.enterScope();
    
    // Check local variables
    if (!checkVarList(mainProg->locals)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Check algorithm
    if (!checkStatementList(mainProg->statements)) {
        symbolTable.exitScope();
        return false;
    }
    
    // Exit main scope
    symbolTable.exitScope();
    
    return true;
}

bool TypeChecker::checkStatementList(AstNodeList<StatementNode>* statements) {
    if (!statements) {
        return true; // Empty list is correctly typed
    }
    
    // ALGO is correctly typed if each INSTR is correctly typed
    for (auto* stmt : statements->elements) {
        Type stmtType = checkStatement(stmt);
        if (stmtType == Type::UNKNOWN) {
            return false;
        }
    }
    
    return true;
}

Type TypeChecker::checkStatement(StatementNode* stmt) {
    if (!stmt) {
        addError("Statement node is null");
        return Type::UNKNOWN;
    }
    
    // Check based on statement type
    if (auto* halt = dynamic_cast<HaltNode*>(stmt)) {
        return checkHalt(halt) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* print = dynamic_cast<PrintNode*>(stmt)) {
        return checkPrint(print) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* procCall = dynamic_cast<ProcCallNode*>(stmt)) {
        return checkProcCall(procCall) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* assign = dynamic_cast<AssignNode*>(stmt)) {
        return checkAssign(assign) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* ifNode = dynamic_cast<IfNode*>(stmt)) {
        return checkIf(ifNode) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* ifElseNode = dynamic_cast<IfElseNode*>(stmt)) {
        return checkIfElse(ifElseNode) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* whileNode = dynamic_cast<WhileNode*>(stmt)) {
        return checkWhile(whileNode) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* doUntilNode = dynamic_cast<DoUntilNode*>(stmt)) {
        return checkDoUntil(doUntilNode) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else if (auto* returnNode = dynamic_cast<ReturnNode*>(stmt)) {
        return checkReturn(returnNode) ? Type::NUMERIC : Type::UNKNOWN;
    }
    else {
        addError("Unknown statement type");
        return Type::UNKNOWN;
    }
}

bool TypeChecker::checkHalt(HaltNode* halt) {
    // INSTR ::= halt is correctly typed (fact)
    return true;
}

bool TypeChecker::checkPrint(PrintNode* print) {
    if (!print) {
        addError("Print node is null");
        return false;
    }
    
    // INSTR ::= print OUTPUT is correctly typed if OUTPUT is correctly typed
    return checkOutput(print->expression);
}

bool TypeChecker::checkOutput(ExpressionNode* output) {
    if (!output) {
        addError("Output node is null");
        return false;
    }
    
    // OUTPUT ::= ATOM is correctly typed if ATOM is of type "numeric"
    // OUTPUT ::= string is correctly typed (fact)
    
    if (auto* stringNode = dynamic_cast<StringNode*>(output)) {
        return true; // String output is always correctly typed
    }
    else {
        Type atomType = checkAtom(output);
        return atomType == Type::NUMERIC;
    }
}

bool TypeChecker::checkProcCall(ProcCallNode* procCall) {
    if (!procCall) {
        addError("ProcCall node is null");
        return false;
    }
    
    // INSTR ::= NAME (INPUT) is correctly typed if NAME is type-less and INPUT is correctly typed
    
    // Check if name is type-less (declared as procedure)
    if (!symbolTable.isDeclared(procCall->name)) {
        addError("Procedure " + procCall->name + " is not declared");
        return false;
    }
    
    // Check input
    return checkInput(procCall->args);
}

bool TypeChecker::checkInput(AstNodeList<ExpressionNode>* input) {
    if (!input) {
        return true; // Empty input is correctly typed (fact)
    }
    
    // INPUT is correctly typed if each ATOM is of type "numeric"
    for (auto* expr : input->elements) {
        Type atomType = checkAtom(expr);
        if (atomType != Type::NUMERIC) {
            addError("Input argument is not of type numeric");
            return false;
        }
    }
    
    return true;
}

bool TypeChecker::checkAssign(AssignNode* assign) {
    if (!assign) {
        addError("Assign node is null");
        return false;
    }
    
    // ASSIGN ::= VAR = NAME (INPUT) is correctly typed if NAME is type-less, INPUT is correctly typed, and VAR is of type "numeric"
    // ASSIGN ::= VAR = TERM is correctly typed if TERM is of type "numeric" and VAR is of type "numeric"
    
    // Check if VAR is of type "numeric"
    if (!symbolTable.isDeclared(assign->var->name)) {
        addError("Variable " + assign->var->name + " is not declared");
        return false;
    }
    
    if (symbolTable.getType(assign->var->name) != Type::NUMERIC) {
        addError("Variable " + assign->var->name + " is not of type numeric");
        return false;
    }
    
    // Check right-hand side
    Type rhsType = checkExpression(assign->expression);
    if (rhsType != Type::NUMERIC) {
        addError("Assignment right-hand side is not of type numeric");
        return false;
    }
    
    return true;
}

Type TypeChecker::checkExpression(ExpressionNode* expr) {
    if (!expr) {
        addError("Expression node is null");
        return Type::UNKNOWN;
    }
    
    // Check based on expression type
    if (auto* var = dynamic_cast<VarNode*>(expr)) {
        return checkVar(var);
    }
    else if (auto* number = dynamic_cast<NumberNode*>(expr)) {
        return Type::NUMERIC; // Number is of type "numeric" (fact)
    }
    else if (auto* string = dynamic_cast<StringNode*>(expr)) {
        return Type::NUMERIC; // String is of type "numeric" (fact)
    }
    else if (auto* unaryOp = dynamic_cast<UnaryOpNode*>(expr)) {
        return checkUnaryOp(unaryOp);
    }
    else if (auto* binaryOp = dynamic_cast<BinaryOpNode*>(expr)) {
        return checkBinaryOp(binaryOp);
    }
    else if (auto* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        return checkFuncCall(funcCall);
    }
    else {
        addError("Unknown expression type");
        return Type::UNKNOWN;
    }
}

Type TypeChecker::checkAtom(ExpressionNode* atom) {
    if (!atom) {
        addError("Atom node is null");
        return Type::UNKNOWN;
    }
    
    // ATOM ::= VAR is of type "numeric" if VAR is of type "numeric"
    // ATOM ::= number is of type "numeric" (fact)
    
    if (auto* var = dynamic_cast<VarNode*>(atom)) {
        if (!symbolTable.isDeclared(var->name)) {
            addError("Variable " + var->name + " is not declared");
            return Type::UNKNOWN;
        }
        return symbolTable.getType(var->name);
    }
    else if (auto* number = dynamic_cast<NumberNode*>(atom)) {
        return Type::NUMERIC; // Number is of type "numeric" (fact)
    }
    else {
        addError("Invalid atom type");
        return Type::UNKNOWN;
    }
}

Type TypeChecker::checkTerm(ExpressionNode* term) {
    if (!term) {
        addError("Term node is null");
        return Type::UNKNOWN;
    }
    
    // TERM ::= ATOM is of type "numeric" if ATOM is of type "numeric"
    // TERM ::= (UNOP TERM) is of type "numeric" if UNOP is of type "numeric" and TERM is of type "numeric"
    // TERM ::= (UNOP TERM) is of type "boolean" if UNOP is of type "boolean" and TERM is of type "boolean"
    // TERM ::= (TERM BINOP TERM) is of type "numeric" if BINOP is of type "numeric" and both TERM are of type "numeric"
    // TERM ::= (TERM BINOP TERM) is of type "boolean" if BINOP is of type "boolean" and both TERM are of type "boolean"
    // TERM ::= (TERM BINOP TERM) is of type "boolean" if BINOP is of type "comparison" and both TERM are of type "numeric"
    
    if (auto* atom = dynamic_cast<VarNode*>(term)) {
        return checkAtom(atom);
    }
    else if (auto* number = dynamic_cast<NumberNode*>(term)) {
        return checkAtom(number);
    }
    else if (auto* unaryOp = dynamic_cast<UnaryOpNode*>(term)) {
        return checkUnaryOp(unaryOp);
    }
    else if (auto* binaryOp = dynamic_cast<BinaryOpNode*>(term)) {
        return checkBinaryOp(binaryOp);
    }
    else {
        addError("Invalid term type");
        return Type::UNKNOWN;
    }
}

Type TypeChecker::checkUnaryOp(UnaryOpNode* unaryOp) {
    if (!unaryOp) {
        addError("UnaryOp node is null");
        return Type::UNKNOWN;
    }
    
    // UNOP ::= neg is of type "numeric" (fact)
    // UNOP ::= not is of type "boolean" (fact)
    
    Type operandType = checkTerm(unaryOp->operand);
    
    if (unaryOp->op == "neg") {
        if (operandType != Type::NUMERIC) {
            addError("Negation operand must be of type numeric");
            return Type::UNKNOWN;
        }
        return Type::NUMERIC;
    }
    else if (unaryOp->op == "not") {
        if (operandType != Type::BOOLEAN) {
            addError("Not operand must be of type boolean");
            return Type::UNKNOWN;
        }
        return Type::BOOLEAN;
    }
    else {
        addError("Unknown unary operator: " + unaryOp->op);
        return Type::UNKNOWN;
    }
}

Type TypeChecker::checkBinaryOp(BinaryOpNode* binaryOp) {
    if (!binaryOp) {
        addError("BinaryOp node is null");
        return Type::UNKNOWN;
    }
    
    Type leftType = checkTerm(binaryOp->left);
    Type rightType = checkTerm(binaryOp->right);
    
    // BINOP ::= > is of type "comparison" (fact)
    // BINOP ::= eq is of type "comparison" (fact)
    // BINOP ::= or is of type "boolean" (fact)
    // BINOP ::= and is of type "boolean" (fact)
    // BINOP ::= plus is of type "numeric" (fact)
    // BINOP ::= minus is of type "numeric" (fact)
    // BINOP ::= mult is of type "numeric" (fact)
    // BINOP ::= div is of type "numeric" (fact)
    
    if (binaryOp->op == ">" || binaryOp->op == "eq") {
        // Comparison operators
        if (leftType != Type::NUMERIC || rightType != Type::NUMERIC) {
            addError("Comparison operands must be of type numeric");
            return Type::UNKNOWN;
        }
        return Type::BOOLEAN;
    }
    else if (binaryOp->op == "or" || binaryOp->op == "and") {
        // Boolean operators
        if (leftType != Type::BOOLEAN || rightType != Type::BOOLEAN) {
            addError("Boolean operands must be of type boolean");
            return Type::UNKNOWN;
        }
        return Type::BOOLEAN;
    }
    else if (binaryOp->op == "plus" || binaryOp->op == "minus" || 
             binaryOp->op == "mult" || binaryOp->op == "div") {
        // Numeric operators
        if (leftType != Type::NUMERIC || rightType != Type::NUMERIC) {
            addError("Numeric operands must be of type numeric");
            return Type::UNKNOWN;
        }
        return Type::NUMERIC;
    }
    else {
        addError("Unknown binary operator: " + binaryOp->op);
        return Type::UNKNOWN;
    }
}

Type TypeChecker::checkFuncCall(FuncCallNode* funcCall) {
    if (!funcCall) {
        addError("FuncCall node is null");
        return Type::UNKNOWN;
    }
    
    // Function calls return numeric type
    // Check if function is declared
    if (!symbolTable.isDeclared(funcCall->name)) {
        addError("Function " + funcCall->name + " is not declared");
        return Type::UNKNOWN;
    }
    
    // Check input arguments
    if (!checkInput(funcCall->args)) {
        return Type::UNKNOWN;
    }
    
    return Type::NUMERIC;
}

bool TypeChecker::checkIf(IfNode* ifNode) {
    if (!ifNode) {
        addError("If node is null");
        return false;
    }
    
    // BRANCH ::= if TERM { ALGO } is correctly typed if TERM is of type "boolean" and ALGO is correctly typed
    
    Type conditionType = checkTerm(ifNode->condition);
    if (conditionType != Type::BOOLEAN) {
        addError("If condition must be of type boolean");
        return false;
    }
    
    return checkStatementList(ifNode->then_branch);
}

bool TypeChecker::checkIfElse(IfElseNode* ifElseNode) {
    if (!ifElseNode) {
        addError("IfElse node is null");
        return false;
    }
    
    // BRANCH ::= if TERM { ALGO } else { ALGO } is correctly typed if TERM is of type "boolean" and both ALGO are correctly typed
    
    Type conditionType = checkTerm(ifElseNode->condition);
    if (conditionType != Type::BOOLEAN) {
        addError("If condition must be of type boolean");
        return false;
    }
    
    bool thenOk = checkStatementList(ifElseNode->then_branch);
    bool elseOk = checkStatementList(ifElseNode->else_branch);
    
    return thenOk && elseOk;
}

bool TypeChecker::checkWhile(WhileNode* whileNode) {
    if (!whileNode) {
        addError("While node is null");
        return false;
    }
    
    // LOOP ::= while TERM { ALGO } is correctly typed if TERM is of type "boolean" and ALGO is correctly typed
    
    Type conditionType = checkTerm(whileNode->condition);
    if (conditionType != Type::BOOLEAN) {
        addError("While condition must be of type boolean");
        return false;
    }
    
    return checkStatementList(whileNode->body);
}

bool TypeChecker::checkDoUntil(DoUntilNode* doUntilNode) {
    if (!doUntilNode) {
        addError("DoUntil node is null");
        return false;
    }
    
    // LOOP ::= do { ALGO } until TERM is correctly typed if TERM is of type "boolean" and ALGO is correctly typed
    
    Type conditionType = checkTerm(doUntilNode->condition);
    if (conditionType != Type::BOOLEAN) {
        addError("Do-until condition must be of type boolean");
        return false;
    }
    
    return checkStatementList(doUntilNode->body);
}

bool TypeChecker::checkReturn(ReturnNode* returnNode) {
    if (!returnNode) {
        addError("Return node is null");
        return false;
    }
    
    // Return statement should return a numeric value
    Type returnType = checkAtom(returnNode->expression);
    if (returnType != Type::NUMERIC) {
        addError("Return value must be of type numeric");
        return false;
    }
    
    return true;
}

bool TypeChecker::hasTypeErrors() const {
    return hasErrors;
}

const std::vector<std::string>& TypeChecker::getErrorMessages() const {
    return errorMessages;
}

void TypeChecker::printErrors() const {
    if (hasErrors) {
        //std::cout << "\n=== Type Checker Errors ===" << std::endl;
        for (const auto& error : errorMessages) {
            std::cout << "Type error: " << error << std::endl;
        }
        //std::cout << "===========================" << std::endl;
    } else {
        //std::cout << "\n=== Type Checker: No errors found ===" << std::endl;
    }
}

void TypeChecker::printSymbolTable() const {
    symbolTable.printSymbols();
}

std::unordered_map<std::string, Type> SymbolTable::getSymbols() const {
    return symbols;
}

const std::vector<std::unordered_map<std::string, Type>>& SymbolTable::getScopes() const {
    return scopes;
}
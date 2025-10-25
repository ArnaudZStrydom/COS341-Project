#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <unordered_set>
#include <regex>

static const std::unordered_set<std::string> RESERVED_KEYWORDS = {
    "glob","proc","func","main","return","local","var","halt","print",
    "while","do","until","if","else",
    "neg","not","eq",">","or","and","plus","minus","mult","div"
};

static const std::regex IDENT_REGEX("^[a-z][a-z0-9]*$");
static const std::regex NUMBER_REGEX("^(0|[1-9][0-9]*)$");
static const std::regex STRING_REGEX("^[A-Za-z0-9]{0,15}$");

inline bool checkIdentifier(const std::string& name) {
    if (RESERVED_KEYWORDS.count(name)) {
        std::cerr << "Invalid identifier: '" << name << "' is a reserved keyword." << std::endl;
        return false;
    }
    if (!std::regex_match(name, IDENT_REGEX)) {
        std::cerr << "Invalid identifier: '" << name << "'. Must match [a-z][a-z0-9]*" << std::endl;
        return false;
    }
    return true;
}

inline bool checkNumber(const std::string& value) {
    if (!std::regex_match(value, NUMBER_REGEX)) {
        std::cerr << "Invalid number constant: '" << value << "'" << std::endl;
        return false;
    }
    return true;
}

inline bool checkString(const std::string& value) {
    if (value.length() > 15) {
        std::cerr << "String literal exceeds 15 characters: '" << value << "'" << std::endl;
        return false;
    }
    if (!std::regex_match(value, STRING_REGEX)) {
        std::cerr << "Invalid string literal: '" << value << "'. Only letters/digits allowed." << std::endl;
        return false;
    }
    return true;
}


// A helper function for indentation when printing the tree
inline void print_indent(int indent) {
    std::cout << std::string(indent, ' ');
}

// ==================================================================
// Base Classes
// ==================================================================

class AstNode {
public:
    virtual ~AstNode() = default;
    virtual void print(int indent = 0) const = 0;
    virtual void checkNames() const = 0;
};

class StatementNode : public AstNode {};
class ExpressionNode : public AstNode {};

template<typename T>
class AstNodeList : public AstNode {
public:
    std::vector<T*> elements;
    ~AstNodeList() {
        for (auto* element : elements) {
            delete element;
        }
    }
    void print(int indent = 0) const override {
        for (const auto* element : elements) {
            element->print(indent);
        }
    }

    void checkNames() const override{
        for (const auto* element : elements) {
            element->checkNames();
        }
    }

};


// ==================================================================
// Expression Nodes
// ==================================================================

class VarNode : public ExpressionNode {
public:
    std::string name;
    VarNode(const std::string& name) : name(name) {}
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Var(" << name << ")" << std::endl;
    }

    void checkNames() const override {
        checkIdentifier(name);
    }

};

class NumberNode : public ExpressionNode {
public:
    std::string value;
    NumberNode(const std::string& value) : value(value) {}
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Number(" << value << ")" << std::endl;
    }

    void checkNames() const override {
        checkNumber(value);
    }


};

class StringNode : public ExpressionNode {
public:
    std::string value;
    StringNode(const std::string& value) : value(value) {}
     void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "String(\"" << value << "\")" << std::endl;
    }

    void checkNames() const override {
        checkString(value);
    }
};

class UnaryOpNode : public ExpressionNode {
public:
    std::string op;
    ExpressionNode* operand;
    UnaryOpNode(const std::string& op, ExpressionNode* operand) : op(op), operand(operand) {}
    ~UnaryOpNode() { delete operand; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "UnaryOp(" << op << ")" << std::endl;
        operand->print(indent + 2);
    }

    void checkNames() const override {
        if(operand) operand->checkNames();
    }
};

class BinaryOpNode : public ExpressionNode {
public:
    ExpressionNode* left;
    std::string op;
    ExpressionNode* right;
    BinaryOpNode(ExpressionNode* left, const std::string& op, ExpressionNode* right) : left(left), op(op), right(right) {}
    ~BinaryOpNode() { delete left; delete right; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "BinaryOp(" << op << ")" << std::endl;
        left->print(indent + 2);
        right->print(indent + 2);
    }

    void checkNames() const override {
        if(left) left->checkNames();
        if(right) right->checkNames();
    }
};

class FuncCallNode : public ExpressionNode {
public:
    std::string name;
    AstNodeList<ExpressionNode>* args;
    FuncCallNode(const std::string& name, AstNodeList<ExpressionNode>* args) : name(name), args(args) {}
    ~FuncCallNode() { delete args; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "FuncCall(" << name << ")" << std::endl;
        if(args) args->print(indent + 2);
    }

    void checkNames() const override {
        checkIdentifier(name);
        if (args) args->checkNames();
    }
};


// ==================================================================
// Statement Nodes and Definitions
// ==================================================================

class HaltNode : public StatementNode {
public:
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Halt" << std::endl;
    }

    void checkNames() const override {
    }
};

class PrintNode : public StatementNode {
public:
    ExpressionNode* expression;
    PrintNode(ExpressionNode* expr) : expression(expr) {}
    ~PrintNode() { delete expression; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Print" << std::endl;
        expression->print(indent + 2);
    }
    void checkNames() const override {
        if(expression) expression->checkNames();
    }
};

class ProcCallNode : public StatementNode {
public:
    std::string name;
    AstNodeList<ExpressionNode>* args;
    ProcCallNode(const std::string& name, AstNodeList<ExpressionNode>* args) : name(name), args(args) {}
    ~ProcCallNode() { delete args; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "ProcCall(" << name << ")" << std::endl;
        if(args) args->print(indent + 2);
    }

    void checkNames() const override {
        checkIdentifier(name);
        if (args) args->checkNames();
    }
};

class AssignNode : public StatementNode {
public:
    VarNode* var;
    ExpressionNode* expression;
    AssignNode(VarNode* var, ExpressionNode* expr) : var(var), expression(expr) {}
    ~AssignNode() { delete var; delete expression; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Assign" << std::endl;
        var->print(indent + 2);
        expression->print(indent + 2);
    }

    void checkNames() const override {
        if (var) var->checkNames();
        if (expression) expression->checkNames();
    }

};

class IfNode : public StatementNode {
public:
    ExpressionNode* condition;
    AstNodeList<StatementNode>* then_branch;
    IfNode(ExpressionNode* cond, AstNodeList<StatementNode>* then_b) : condition(cond), then_branch(then_b) {}
    ~IfNode() { delete condition; delete then_branch; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "If" << std::endl;
        condition->print(indent + 2);
        print_indent(indent);
        std::cout << "Then" << std::endl;
        then_branch->print(indent + 2);
    }

    void checkNames() const override {
        if(condition) condition->checkNames();
        if(then_branch) then_branch->checkNames();
    }
};

class IfElseNode : public StatementNode {
public:
    ExpressionNode* condition;
    AstNodeList<StatementNode>* then_branch;
    AstNodeList<StatementNode>* else_branch;
    IfElseNode(ExpressionNode* cond, AstNodeList<StatementNode>* then_b, AstNodeList<StatementNode>* else_b) 
        : condition(cond), then_branch(then_b), else_branch(else_b) {}
    ~IfElseNode() { delete condition; delete then_branch; delete else_branch; }
     void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "IfElse" << std::endl;
        condition->print(indent + 2);
        print_indent(indent);
        std::cout << "Then" << std::endl;
        then_branch->print(indent + 2);
        print_indent(indent);
        std::cout << "Else" << std::endl;
        else_branch->print(indent + 2);
    }

    void checkNames() const override {
        if(condition) condition->checkNames();
        if(then_branch) then_branch->checkNames();
        if(else_branch) else_branch->checkNames();
    }
};

class WhileNode : public StatementNode {
public:
    ExpressionNode* condition;
    AstNodeList<StatementNode>* body;
    WhileNode(ExpressionNode* cond, AstNodeList<StatementNode>* b) : condition(cond), body(b) {}
    ~WhileNode() { delete condition; delete body; }
     void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "While" << std::endl;
        condition->print(indent + 2);
        print_indent(indent);
        std::cout << "Body" << std::endl;
        body->print(indent + 2);
    }

    void checkNames() const override {
        if(condition) condition->checkNames();
        if(body) body->checkNames();
    }
};

class DoUntilNode : public StatementNode {
public:
    AstNodeList<StatementNode>* body;
    ExpressionNode* condition;
    DoUntilNode(AstNodeList<StatementNode>* b, ExpressionNode* cond) : body(b), condition(cond) {}
    ~DoUntilNode() { delete body; delete condition; }
     void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "DoUntil" << std::endl;
        print_indent(indent);
        std::cout << "Body" << std::endl;
        body->print(indent + 2);
        condition->print(indent + 2);
    }
    
    void checkNames() const override {
        if(body) body->checkNames();
        if(condition) condition->checkNames();
    }
};

class BodyNode : public AstNode {
public:
    AstNodeList<VarNode>* locals;
    AstNodeList<StatementNode>* statements;
    BodyNode(AstNodeList<VarNode>* l, AstNodeList<StatementNode>* s) : locals(l), statements(s) {}
    ~BodyNode() { delete locals; delete statements; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Body" << std::endl;
        locals->print(indent + 2);
        statements->print(indent + 2);
    }

    void checkNames() const override {
        if (locals) locals->checkNames();
        if (statements) statements->checkNames();
    }

};

class ProcDefNode : public AstNode {
public:
    std::string name;
    AstNodeList<VarNode>* params;
    BodyNode* body;
    ProcDefNode(const std::string& n, AstNodeList<VarNode>* p, BodyNode* b) : name(n), params(p), body(b) {}
    ~ProcDefNode() { delete params; delete body; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "ProcDef(" << name << ")" << std::endl;
        params->print(indent + 2);
        body->print(indent + 2);
    }

    void checkNames() const override {
        checkIdentifier(name);
        if (params) params->checkNames();
        if (body) body->checkNames();
    }
};

class FuncDefNode : public AstNode {
public:
    std::string name;
    AstNodeList<VarNode>* params;
    BodyNode* body;


    // Update the constructor
    FuncDefNode(const std::string& n, AstNodeList<VarNode>* p, BodyNode* b) 
        : name(n), params(p), body(b) {} 
    
    ~FuncDefNode() { 
        delete params; 
        delete body; 
    }

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "FuncDef(" << name << ")" << std::endl;
        params->print(indent + 2);
        body->print(indent + 2);
    }

    void checkNames() const override {
        checkIdentifier(name);
        if (params) params->checkNames();
        if (body) body->checkNames();
    }
};

class MainProgNode : public AstNode {
public:
    AstNodeList<VarNode>* locals;
    AstNodeList<StatementNode>* statements;
    MainProgNode(AstNodeList<VarNode>* l, AstNodeList<StatementNode>* s) : locals(l), statements(s) {}
    ~MainProgNode() { delete locals; delete statements; }
     void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Main" << std::endl;
        locals->print(indent + 2);
        statements->print(indent + 2);
    }

    void checkNames() const override {
        if (locals) locals->checkNames();
        if (statements) statements->checkNames();
    }
};

class ProgramNode : public AstNode {
public:
    AstNodeList<VarNode>* globals;
    AstNodeList<ProcDefNode>* procs;
    AstNodeList<FuncDefNode>* funcs;
    MainProgNode* main;
    ProgramNode(AstNodeList<VarNode>* g, AstNodeList<ProcDefNode>* p, AstNodeList<FuncDefNode>* f, MainProgNode* m)
        : globals(g), procs(p), funcs(f), main(m) {}
    ~ProgramNode() { delete globals; delete procs; delete funcs; delete main; }
     void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Program" << std::endl;
        globals->print(indent + 2);
        procs->print(indent + 2);
        funcs->print(indent + 2);
        main->print(indent + 2);
    }

    void checkNames() const override {
        if (globals) globals->checkNames();
        if (procs) procs->checkNames();
        if (funcs) funcs->checkNames();
        if (main) main->checkNames();
    }
};

class ReturnNode : public StatementNode {
public:
    ExpressionNode* expression;
    ReturnNode(ExpressionNode* expr) : expression(expr) {}
    ~ReturnNode() { delete expression; }
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Return" << std::endl;
        expression->print(indent + 2);
    }

    void checkNames() const override {
        if(expression) expression->checkNames();
    }
};



#endif // AST_H
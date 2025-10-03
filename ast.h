#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

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
};

class NumberNode : public ExpressionNode {
public:
    std::string value;
    NumberNode(const std::string& value) : value(value) {}
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Number(" << value << ")" << std::endl;
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
};

#endif // AST_H
#include "../ast.h"
#include <sstream>
#include <iostream>
#include <string>

static int temp_counter = 0;
static int label_counter = 0;

extern std::string newTemp() { return "t" + std::to_string(temp_counter++); }
extern std::string newLabel() { return "L" + std::to_string(label_counter++); }

std::string generateExpr(ExpressionNode* expr);

std::string generateVar(VarNode* node) {
    return node->name;
}

std::string generateNumber(NumberNode* node) {
    std::string t = newTemp();
    std::cout << t << " = " << node->value << std::endl;
    return t;
}

std::string generateString(StringNode* node) {
    std::string t = newTemp();
    std::cout << t << " = \"" << node->value << "\"" << std::endl;
    return t;
}

std::string generateUnaryOp(UnaryOpNode* node) {
    std::string val = generateExpr(node->operand);
    std::string t = newTemp();
    std::cout << t << " = " << node->op << " " << val << std::endl;
    return t;
}

std::string generateBinaryOp(BinaryOpNode* node) {
    std::string leftVal = generateExpr(node->left);
    std::string rightVal = generateExpr(node->right);
    std::string t = newTemp();
    std::cout << t << " = " << leftVal << " " << node->op << " " << rightVal << std::endl;
    return t;
}

std::string generateFuncCall(FuncCallNode* node) {
    if (node->args) {
        for (auto* arg : node->args->elements) {
            std::string val = generateExpr(arg);
            std::cout << "param " << val << std::endl;
        }
    }
    std::string t = newTemp();
    std::cout << t << " = call " << node->name << ", " 
              << (node->args ? node->args->elements.size() : 0) << std::endl;
    return t;
}

std::string generateExpr(ExpressionNode* expr) {
    if (auto* n = dynamic_cast<VarNode*>(expr)) return generateVar(n);
    if (auto* n = dynamic_cast<NumberNode*>(expr)) return generateNumber(n);
    if (auto* n = dynamic_cast<StringNode*>(expr)) return generateString(n);
    if (auto* n = dynamic_cast<UnaryOpNode*>(expr)) return generateUnaryOp(n);
    if (auto* n = dynamic_cast<BinaryOpNode*>(expr)) return generateBinaryOp(n);
    if (auto* n = dynamic_cast<FuncCallNode*>(expr)) return generateFuncCall(n);

    std::cerr << "Unknown expression type in codegen." << std::endl;
    return "ERROR";
}

void generateStmt(StatementNode* stmt);
void generateBody(BodyNode* body);
void generateStatements(AstNodeList<StatementNode>* list);

void generateAssign(AssignNode* node) {
    std::string rhs = generateExpr(node->expression);
    std::cout << node->var->name << " = " << rhs << std::endl;
}

void generatePrint(PrintNode* node) {
    std::string val = generateExpr(node->expression);
    std::cout << "print " << val << std::endl;
}

void generateProcCall(ProcCallNode* node) {
    if (node->args) {
        for (auto* arg : node->args->elements) {
            std::string val = generateExpr(arg);
            std::cout << "param " << val << std::endl;
        }
    }
    std::cout << "call " << node->name << ", "
              << (node->args ? node->args->elements.size() : 0) << std::endl;
}

void generateIf(IfNode* node) {
    std::string cond = generateExpr(node->condition);
    std::string Ltrue = newLabel();
    std::string Lend = newLabel();
    std::cout << "if " << cond << " goto " << Ltrue << std::endl;
    std::cout << "goto " << Lend << std::endl;
    std::cout << Ltrue << ":" << std::endl;
    generateStatements(node->then_branch);
    std::cout << Lend << ":" << std::endl;
}

void generateIfElse(IfElseNode* node) {
    std::string cond = generateExpr(node->condition);
    std::string Lthen = newLabel();
    std::string Lelse = newLabel();
    std::string Lend = newLabel();

    std::cout << "if " << cond << " goto " << Lthen << std::endl;
    std::cout << "goto " << Lelse << std::endl;
    std::cout << Lthen << ":" << std::endl;
    generateStatements(node->then_branch);
    std::cout << "goto " << Lend << std::endl;
    std::cout << Lelse << ":" << std::endl;
    generateStatements(node->else_branch);
    std::cout << Lend << ":" << std::endl;
}

void generateWhile(WhileNode* node) {
    std::string Lbegin = newLabel();
    std::string Lend = newLabel();
    std::cout << Lbegin << ":" << std::endl;
    std::string cond = generateExpr(node->condition);
    std::cout << "ifFalse " << cond << " goto " << Lend << std::endl;
    generateStatements(node->body);
    std::cout << "goto " << Lbegin << std::endl;
    std::cout << Lend << ":" << std::endl;
}

void generateDoUntil(DoUntilNode* node) {
    std::string Lbegin = newLabel();
    std::cout << Lbegin << ":" << std::endl;
    generateStatements(node->body);
    std::string cond = generateExpr(node->condition);
    std::cout << "ifFalse " << cond << " goto " << Lbegin << std::endl;
}

void generateReturn(ReturnNode* node) {
    std::string val = generateExpr(node->expression);
    std::cout << "return " << val << std::endl;
}

void generateHalt(HaltNode*) {
    std::cout << "halt" << std::endl;
}

void generateStmt(StatementNode* stmt) {
    if (auto* n = dynamic_cast<AssignNode*>(stmt)) return generateAssign(n);
    if (auto* n = dynamic_cast<PrintNode*>(stmt)) return generatePrint(n);
    if (auto* n = dynamic_cast<ProcCallNode*>(stmt)) return generateProcCall(n);
    if (auto* n = dynamic_cast<IfNode*>(stmt)) return generateIf(n);
    if (auto* n = dynamic_cast<IfElseNode*>(stmt)) return generateIfElse(n);
    if (auto* n = dynamic_cast<WhileNode*>(stmt)) return generateWhile(n);
    if (auto* n = dynamic_cast<DoUntilNode*>(stmt)) return generateDoUntil(n);
    if (auto* n = dynamic_cast<ReturnNode*>(stmt)) return generateReturn(n);
    if (auto* n = dynamic_cast<HaltNode*>(stmt)) return generateHalt(n);

    std::cerr << "Unknown statement type in codegen." << std::endl;
}

void generateStatements(AstNodeList<StatementNode>* list) {
    for (auto* s : list->elements)
        generateStmt(s);
}

void generateBody(BodyNode* body) {
    std::cout << "# Body begin" << std::endl;
    if (body->locals)
        for (auto* v : body->locals->elements)
            std::cout << "decl " << v->name << std::endl;
    generateStatements(body->statements);
    std::cout << "# Body end" << std::endl;
}

void generateProcDef(ProcDefNode* node) {
    std::cout << "proc " << node->name << ":" << std::endl;
    if (node->params)
        for (auto* p : node->params->elements)
            std::cout << "param " << p->name << std::endl;
    generateBody(node->body);
    std::cout << "endproc" << std::endl;
}

void generateFuncDef(FuncDefNode* node) {
    std::cout << "func " << node->name << ":" << std::endl;
    if (node->params)
        for (auto* p : node->params->elements)
            std::cout << "param " << p->name << std::endl;
    generateBody(node->body);
    std::cout << "endfunc" << std::endl;
}

void generateMain(MainProgNode* node) {
    std::cout << "main:" << std::endl;
    if (node->locals)
        for (auto* v : node->locals->elements)
            std::cout << "decl " << v->name << std::endl;
    generateStatements(node->statements);
    std::cout << "endmain" << std::endl;
}

void generateProgram(ProgramNode* prog) {
    std::cout << "# Program Start" << std::endl;

    if (prog->globals)
        for (auto* g : prog->globals->elements)
            std::cout << "global " << g->name << std::endl;

    if (prog->procs)
        for (auto* p : prog->procs->elements)
            generateProcDef(p);

    if (prog->funcs)
        for (auto* f : prog->funcs->elements)
            generateFuncDef(f);

    generateMain(prog->main);
    std::cout << "# Program End" << std::endl;
}


void generateIntermediateCode(ProgramNode* root) {
    generateProgram(root);
}

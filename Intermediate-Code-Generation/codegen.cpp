#include "codegen.h"
#include <sstream>
#include <fstream>
#include <iostream>

// =================== PUBLIC ===================

void CodeGen::generate(ProgramNode* program) {
    if (!program) return;

    std::string generatedCode = genProgram(program);

    std::stringstream ss(generatedCode);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty())
            code.push_back(line);
    }
}

void CodeGen::saveCode() const {
    std::ofstream outputFile("ICG.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Could not open ICG.txt for writing" << std::endl;
        return;
    }
    for (const auto& line : code) {
        outputFile << line << std::endl;
    }
    outputFile.close();
}

// =================== PRIVATE ===================

std::string CodeGen::genProgram(ProgramNode* program) {
    if (!program) return "";

    std::stringstream ss;

    // Procedures
    if (program->procs) {
        for (auto* proc : program->procs->elements) {
            ss << "SUB " << proc->name << "()" << "\n";
            ss << genStatementList(proc->body->statements);
            ss << "END SUB\n\n";
        }
    }

    // Functions
    if (program->funcs) {
        for (auto* func : program->funcs->elements) {
            ss << "FUNCTION " << func->name << "()" << "\n";
            ss << genStatementList(func->body->statements);
            ss << "END FUNCTION\n\n";
        }
    }

    // Main program
    if (program->main) {
        ss << genStatementList(program->main->statements);
    }

    return ss.str();
}

std::string CodeGen::genStatementList(AstNodeList<StatementNode>* stmts) {
    if (!stmts) return "";
    std::stringstream ss;
    for (auto* stmt : stmts->elements) {
        ss << genStatement(stmt) << "\n";
    }
    return ss.str();
}

std::string CodeGen::genStatement(StatementNode* stmt) {
    if (!stmt) return "";

    if (auto* halt = dynamic_cast<HaltNode*>(stmt)) {
        return "STOP";
    } 
    else if (auto* print = dynamic_cast<PrintNode*>(stmt)) {
        return "PRINT " + genExpression(print->expression);
    } 
    else if (auto* assign = dynamic_cast<AssignNode*>(stmt)) {
        return assign->var->name + " = " + genExpression(assign->expression);
    } 
    else if (auto* procCall = dynamic_cast<ProcCallNode*>(stmt)) {
        std::string args;
        if (procCall->args) {
            for (size_t i = 0; i < procCall->args->elements.size(); i++) {
                args += genExpression(procCall->args->elements[i]);
                if (i + 1 < procCall->args->elements.size()) args += ", ";
            }
        }
        return "CALL " + procCall->name + "(" + args + ")";
    } 
    else if (auto* ifNode = dynamic_cast<IfNode*>(stmt)) {
        std::stringstream ss;
        std::string labelT = "LBL_THEN_" + std::to_string(reinterpret_cast<uintptr_t>(ifNode));
        std::string labelExit = "LBL_EXIT_" + std::to_string(reinterpret_cast<uintptr_t>(ifNode));

        ss << "IF " << genExpression(ifNode->condition) << " THEN " << labelT << "\n";
        ss << "GOTO " << labelExit << "\n";
        ss << "REM " << labelT << "\n";
        ss << genStatementList(ifNode->then_branch);
        ss << "REM " << labelExit;

        return ss.str();
    }
    else if (auto* ifElseNode = dynamic_cast<IfElseNode*>(stmt)) {
        std::stringstream ss;
        std::string labelThen = "LBL_THEN_" + std::to_string(reinterpret_cast<uintptr_t>(ifElseNode));
        std::string labelExit = "LBL_EXIT_" + std::to_string(reinterpret_cast<uintptr_t>(ifElseNode));

        ss << "IF " << genExpression(ifElseNode->condition) << " THEN " << labelThen << "\n";
        ss << genStatementList(ifElseNode->else_branch);
        ss << "GOTO " << labelExit << "\n";
        ss << "REM " << labelThen << "\n";
        ss << genStatementList(ifElseNode->then_branch);
        ss << "REM " << labelExit;

        return ss.str();
    }
    else if (auto* whileNode = dynamic_cast<WhileNode*>(stmt)) {
        std::stringstream ss;
        std::string labelStart = "LBL_WHILE_" + std::to_string(reinterpret_cast<uintptr_t>(whileNode));
        std::string labelExit = "LBL_EXIT_WHILE_" + std::to_string(reinterpret_cast<uintptr_t>(whileNode));

        ss << "REM " << labelStart << "\n";
        ss << "IF NOT (" << genExpression(whileNode->condition) << ") THEN " << labelExit << "\n";
        ss << genStatementList(whileNode->body);
        ss << "GOTO " << labelStart << "\n";
        ss << "REM " << labelExit;

        return ss.str();
    }
    else if (auto* doUntilNode = dynamic_cast<DoUntilNode*>(stmt)) {
        std::stringstream ss;
        std::string labelStart = "LBL_DO_" + std::to_string(reinterpret_cast<uintptr_t>(doUntilNode));

        ss << "REM " << labelStart << "\n";
        ss << genStatementList(doUntilNode->body);
        ss << "IF NOT (" << genExpression(doUntilNode->condition) << ") THEN " << labelStart;

        return ss.str();
    }
    else if (auto* returnNode = dynamic_cast<ReturnNode*>(stmt)) {
        return "RETURN " + genExpression(returnNode->expression);
    }

    return "";
}

std::string CodeGen::genExpression(ExpressionNode* expr) {
    if (!expr) return "";

    if (auto* number = dynamic_cast<NumberNode*>(expr)) {
        return number->value;
    } 
    else if (auto* var = dynamic_cast<VarNode*>(expr)) {
        return var->name;
    } 
    else if (auto* stringNode = dynamic_cast<StringNode*>(expr)) {
        return "\"" + stringNode->value + "\"";
    } 
    else if (auto* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        if (unary->op == "neg") return "-" + genExpression(unary->operand);
        // 'not' handled in branching
    } 
    else if (auto* binary = dynamic_cast<BinaryOpNode*>(expr)) {
        std::string op;
        if (binary->op == "plus") op = " + ";
        else if (binary->op == "minus") op = " - ";
        else if (binary->op == "mult") op = " * ";
        else if (binary->op == "div") op = " / ";
        else if (binary->op == "eq") op = " = ";
        else if (binary->op == ">") op = " > ";
        return genExpression(binary->left) + op + genExpression(binary->right);
    } 
    else if (auto* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        std::string args;
        if (funcCall->args) {
            for (size_t i = 0; i < funcCall->args->elements.size(); i++) {
                args += genExpression(funcCall->args->elements[i]);
                if (i + 1 < funcCall->args->elements.size()) args += ", ";
            }
        }
        return funcCall->name + "(" + args + ")";
    }

    return "";
}

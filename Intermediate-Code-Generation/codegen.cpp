#include "codegen.h"
#include <sstream>
#include <fstream>
#include <iostream>

// =================== PUBLIC ===================

void CodeGen::generate(ProgramNode* program) {
    code.clear();
    tempCounter = 0;

    if (!program) return;

    // Generate program and fill code vector directly
    genProgram(program);
}

void CodeGen::saveCode() const {
    std::ofstream outputFile("ICG.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Could not open ICG.txt" << std::endl;
        return;
    }
    for (const auto& line : code) {
        outputFile << line << std::endl;
    }
    outputFile.close();
}

// =================== PRIVATE ===================

std::string CodeGen::newTemp() {
    ++tempCounter;
    return "t" + std::to_string(tempCounter);
}

void CodeGen::emit(const std::string& line) {
    code.push_back(line);
}

std::string CodeGen::genProgram(ProgramNode* program) {
    if (!program) return "";

    // Procedures
    if (program->procs) {
        for (auto* proc : program->procs->elements) {
            emit("SUB " + proc->name + "()");
            // produce body
            genStatementList(proc->body->statements);
            emit("END SUB");
            emit(""); // blank line
        }
    }

    // Functions
    if (program->funcs) {
        for (auto* func : program->funcs->elements) {
            // keep same style as before for header; if you have func->params you can print them here
            emit("FUNCTION " + func->name + "()");
            genStatementList(func->body->statements);
            emit("END FUNCTION");
            emit("");
        }
    }

    // Main program
    if (program->main) {
        genStatementList(program->main->statements);
    }

    return ""; // unused
}

void CodeGen::genStatementList(AstNodeList<StatementNode>* stmts) {
    if (!stmts) return;
    for (auto* stmt : stmts->elements) {
        genStatement(stmt);
    }
}

void CodeGen::genStatement(StatementNode* stmt) {
    if (!stmt) return;

    if (auto* halt = dynamic_cast<HaltNode*>(stmt)) {
        emit("STOP");
    } 
    else if (auto* print = dynamic_cast<PrintNode*>(stmt)) {
        std::string e = genExpression(print->expression);
        emit("PRINT " + e);
    } 
    else if (auto* assign = dynamic_cast<AssignNode*>(stmt)) {
        // Compute RHS into a value (temp or literal/var)
        std::string rhs = genExpression(assign->expression);
        // Direct assignment to lhs
        emit(assign->var->name + " = " + rhs);
    } 
    else if (auto* procCall = dynamic_cast<ProcCallNode*>(stmt)) {
        // Emit params and then a call (no return)
        if (procCall->args) {
            for (size_t i = 0; i < procCall->args->elements.size(); i++) {
                std::string argVal = genExpression(procCall->args->elements[i]);
                emit("PARAM " + argVal);
            }
        }
        emit("CALL " + procCall->name + "()");
    } 
    else if (auto* ifNode = dynamic_cast<IfNode*>(stmt)) {
        std::stringstream ss;
        std::string labelT = "LBL_THEN_" + std::to_string(reinterpret_cast<uintptr_t>(ifNode));
        std::string labelExit = "LBL_EXIT_" + std::to_string(reinterpret_cast<uintptr_t>(ifNode));

        emit("IF " + genExpression(ifNode->condition) + " THEN " + labelT);
        emit("GOTO " + labelExit);
        emit("REM " + labelT);
        genStatementList(ifNode->then_branch);
        emit("REM " + labelExit);
    }
    else if (auto* ifElseNode = dynamic_cast<IfElseNode*>(stmt)) {
        std::string labelThen = "LBL_THEN_" + std::to_string(reinterpret_cast<uintptr_t>(ifElseNode));
        std::string labelExit = "LBL_EXIT_" + std::to_string(reinterpret_cast<uintptr_t>(ifElseNode));

        emit("IF " + genExpression(ifElseNode->condition) + " THEN " + labelThen);
        genStatementList(ifElseNode->else_branch);
        emit("GOTO " + labelExit);
        emit("REM " + labelThen);
        genStatementList(ifElseNode->then_branch);
        emit("REM " + labelExit);
    }
    else if (auto* whileNode = dynamic_cast<WhileNode*>(stmt)) {
        std::string labelStart = "LBL_WHILE_" + std::to_string(reinterpret_cast<uintptr_t>(whileNode));
        std::string labelExit = "LBL_EXIT_WHILE_" + std::to_string(reinterpret_cast<uintptr_t>(whileNode));

        emit("REM " + labelStart);
        emit("IF NOT (" + genExpression(whileNode->condition) + ") THEN " + labelExit);
        genStatementList(whileNode->body);
        emit("GOTO " + labelStart);
        emit("REM " + labelExit);
    }
    else if (auto* doUntilNode = dynamic_cast<DoUntilNode*>(stmt)) {
        std::string labelStart = "LBL_DO_" + std::to_string(reinterpret_cast<uintptr_t>(doUntilNode));

        emit("REM " + labelStart);
        genStatementList(doUntilNode->body);
        emit("IF NOT (" + genExpression(doUntilNode->condition) + ") THEN " + labelStart);
    }
    else if (auto* returnNode = dynamic_cast<ReturnNode*>(stmt)) {
        std::string e = genExpression(returnNode->expression);
        emit("RETURN " + e);
    }
}

std::string CodeGen::genExpression(ExpressionNode* expr) {
    if (!expr) return "";

    // Numbers and vars and strings are returned directly (no temporaries)
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
        if (unary->op == "neg") {
            std::string operand = genExpression(unary->operand);
            // If operand is not a temp, create one to hold it (keeps consistent TF)
            if (operand.size() > 0 && operand[0] != 't') {
                std::string tmp = newTemp();
                emit(tmp + " = " + operand);
                operand = tmp;
            }
            std::string res = newTemp();
            emit(res + " = -" + operand);
            return res;
        }
        // 'not' handled in branching elsewhere
    } 
    else if (auto* binary = dynamic_cast<BinaryOpNode*>(expr)) {
        std::string op;
        if (binary->op == "plus") op = " + ";
        else if (binary->op == "minus") op = " - ";
        else if (binary->op == "mult") op = " * ";
        else if (binary->op == "div") op = " / ";
        else if (binary->op == "eq") op = " = ";
        else if (binary->op == ">") op = " > ";
        else op = " " + binary->op + " ";

        // generate left and right expressions (may emit temps)
        std::string leftVal = genExpression(binary->left);
        std::string rightVal = genExpression(binary->right);

        // ensure left and right are available as temps or direct operands
        std::string leftTemp = leftVal;
        if (leftVal.empty()) leftTemp = "0";
        if (leftVal.size() > 0 && leftVal[0] != 't' && !(leftVal[0] == '"' )) {
            // create a temp holding the left value to mirror your desired IR style
            std::string lt = newTemp();
            emit(lt + " = " + leftVal);
            leftTemp = lt;
        }

        std::string rightTemp = rightVal;
        if (rightVal.empty()) rightTemp = "0";
        if (rightVal.size() > 0 && rightVal[0] != 't' && !(rightVal[0] == '"' )) {
            std::string rt = newTemp();
            emit(rt + " = " + rightVal);
            rightTemp = rt;
        }

        std::string result = newTemp();
        emit(result + " = " + leftTemp + op + rightTemp);
        return result;
    } 
    else if (auto* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        // For function calls inside expressions, emit PARAM lines then a CALL into a temp
        int argc = 0;
        std::string params = "";
        if (funcCall->args) {
            for (size_t i = 0; i < funcCall->args->elements.size(); i++) {
                std::string a = genExpression(funcCall->args->elements[i]);
                params += a + ",";
                //emit("PARAM " + a);
                ++argc;
            }
        }
        std::string result = newTemp();
        params = params.substr(0, params.length() - 1);
        emit(result + " = CALL_" + funcCall->name + "(" + params/* std::to_string(argc)) */ + ")");
        return result;
    }

    return "";
}

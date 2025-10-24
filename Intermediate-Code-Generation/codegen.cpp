#include "codegen.h"
#include <sstream>
#include <fstream>
#include <iostream>

// =================== PUBLIC ===================

void CodeGen::generate(ProgramNode* program) {
    code.clear();
    tempCounter = 0;
    labelCounter = 0;

    if (!program) return;
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

std::string CodeGen::newLabel(const std::string& prefix) {
    ++labelCounter;
    return prefix + "_" + std::to_string(labelCounter);
}

void CodeGen::emit(const std::string& line) {
    code.push_back(line);
}

// ------------------- Program & Statements -------------------

std::string CodeGen::genProgram(ProgramNode* program) {
    if (!program) return "";

    if (program->procs) {
        for (auto* proc : program->procs->elements) {
            emit("PROC " + proc->name + "()");
            genStatementList(proc->body->statements);
            emit("END PROC");
            emit("");
        }
    }

    if (program->funcs) {
        for (auto* func : program->funcs->elements) {
            emit("FUNCTION " + func->name + "()");
            genStatementList(func->body->statements);
            emit("END FUNCTION");
            emit("");
        }
    }

    if (program->main) {
        genStatementList(program->main->statements);
    }

    return "";
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
        std::string rhs = genExpression(assign->expression);
        emit(assign->var->name + " = " + rhs);
    } 
    else if (auto* procCall = dynamic_cast<ProcCallNode*>(stmt)) {
        std::string params = "";
        if (procCall->args) {
            for (auto* a : procCall->args->elements) params += genExpression(a) + ",";
            if (!params.empty()) params.pop_back();
        }
        emit("CALL_" + procCall->name + "(" + params + ")");
    } 
    else if (auto* ifNode = dynamic_cast<IfNode*>(stmt)) {
        std::string labelThen = newLabel("LBL_THEN");
        std::string labelExit = newLabel("LBL_EXIT");
        genCondition(ifNode->condition, labelThen, labelExit);
        emit("REM " + labelThen);
        genStatementList(ifNode->then_branch);
        emit("REM " + labelExit);
    }
    else if (auto* ifElseNode = dynamic_cast<IfElseNode*>(stmt)) {
        std::string labelThen = newLabel("LBL_THEN");
        std::string labelElse = newLabel("LBL_ELSE");
        std::string labelExit = newLabel("LBL_EXIT");

        genCondition(ifElseNode->condition, labelThen, labelElse);

        // Else branch
        emit("REM " + labelElse);
        genStatementList(ifElseNode->else_branch);
        emit("GOTO " + labelExit);

        // Then branch
        emit("REM " + labelThen);
        genStatementList(ifElseNode->then_branch);

        // Exit
        emit("REM " + labelExit);
    }
    else if (auto* whileNode = dynamic_cast<WhileNode*>(stmt)) {
        std::string labelStart = newLabel("LBL_WHILE");
        std::string labelExit = newLabel("LBL_EXIT_WHILE");

        emit("REM " + labelStart);
        genCondition(whileNode->condition, labelStart + "_BODY", labelExit);

        emit("REM " + labelStart + "_BODY");
        genStatementList(whileNode->body);
        emit("GOTO " + labelStart);

        emit("REM " + labelExit);
    }
    else if (auto* doUntilNode = dynamic_cast<DoUntilNode*>(stmt)) {
        std::string labelStart = newLabel("LBL_DO");
        std::string labelExit = newLabel("LBL_EXIT_DO");

        emit("REM " + labelStart);
        genStatementList(doUntilNode->body);
        // Do-until: jump if condition is false
        genCondition(doUntilNode->condition, labelExit, labelStart);
        emit("REM " + labelExit);
    }
    else if (auto* returnNode = dynamic_cast<ReturnNode*>(stmt)) {
        std::string e = genExpression(returnNode->expression);
        emit("RETURN " + e);
    }
}


// ------------------- Expressions -------------------

std::string CodeGen::genExpression(ExpressionNode* expr, bool inCondition) {
    if (!expr) return "";

    if (auto* number = dynamic_cast<NumberNode*>(expr)) {
        return number->value; // constants stay inline
    }

    if (auto* var = dynamic_cast<VarNode*>(expr)) {
        return resolveVariable(var->name); // variables stay inline
    }

    if (auto* stringNode = dynamic_cast<StringNode*>(expr)) {
        return "\"" + stringNode->value + "\"";
    }

    if (auto* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        std::string operand = genExpression(unary->operand);
        std::string tmp = newTemp();

        if (unary->op == "neg") emit(tmp + " = -" + operand);
        else if (unary->op == "not") emit(tmp + " = !" + operand);
        else emit(tmp + " = " + unary->op + " " + operand);

        return tmp;
    }

    if (auto* binary = dynamic_cast<BinaryOpNode*>(expr)) {
        std::string op;
        bool isComparison = false;

        if (binary->op == "plus") op = " + ";
        else if (binary->op == "minus") op = " - ";
        else if (binary->op == "mult") op = " * ";
        else if (binary->op == "div") op = " / ";
        else if (binary->op == "eq") { op = " = "; isComparison = true; }
        else if (binary->op == "ne") { op = " <> "; isComparison = true; }
        else if (binary->op == "gt" || binary->op == ">") { op = " > "; isComparison = true; }
        else if (binary->op == "lt" || binary->op == "<") { op = " < "; isComparison = true; }
        else if (binary->op == "ge") { op = " >= "; isComparison = true; }
        else if (binary->op == "le") { op = " <= "; isComparison = true; }
        else op = " " + binary->op + " ";

        std::string left = genExpression(binary->left);
        std::string right = genExpression(binary->right);

        // always emit temporaries for both operands
        std::string tmpLeft = newTemp();
        emit(tmpLeft + " = " + left);

        std::string tmpRight = newTemp();
        emit(tmpRight + " = " + right);

        // comparisons still get a temporary to hold boolean result
        std::string tmp = newTemp();
        emit(tmp + " = " + tmpLeft + op + tmpRight);

        return tmp;
    }

    if (auto* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        std::string params = "";
        if (funcCall->args) {
            for (auto* a : funcCall->args->elements)
                params += genExpression(a) + ",";
            if (!params.empty()) params.pop_back();
        }
        std::string tmp = newTemp();
        emit(tmp + " = CALL_" + funcCall->name + "(" + params + ")");
        return tmp;
    }

    return "";
}


// ------------------- Conditional Flattening -------------------
void CodeGen::genCondition(ExpressionNode* expr,
                           const std::string& labelTrue,
                           const std::string& labelFalse) {
    if (!expr) return;

    if (auto* binary = dynamic_cast<BinaryOpNode*>(expr)) {
        // Short-circuit AND
        if (binary->op == "and") {
            std::string midLabel = newLabel("LBL_NEXT");
            genCondition(binary->left, midLabel, labelFalse);
            emit("REM " + midLabel);
            genCondition(binary->right, labelTrue, labelFalse);
            return;
        }

        // Short-circuit OR
        if (binary->op == "or") {
            std::string midLabel = newLabel("LBL_NEXT");
            genCondition(binary->left, labelTrue, midLabel);
            emit("REM " + midLabel);
            genCondition(binary->right, labelTrue, labelFalse);
            return;
        }

        // Handle all standard comparisons via temporaries
        std::string left = genExpression(binary->left);
        std::string right = genExpression(binary->right);

        std::string tmpLeft = newTemp();
        emit(tmpLeft + " = " + left);

        std::string tmpRight = newTemp();
        emit(tmpRight + " = " + right);

        std::string op;
        if (binary->op == "eq") op = " = ";
        else if (binary->op == "ne") op = " <> ";
        else if (binary->op == "gt" || binary->op == ">") op = " > ";
        else if (binary->op == "lt" || binary->op == "<") op = " < ";
        else if (binary->op == "ge") op = " >= ";
        else if (binary->op == "le") op = " <= ";
        else op = " " + binary->op + " ";

        emit("IF " + tmpLeft + op + tmpRight + " THEN " + labelTrue);
        emit("GOTO " + labelFalse);
        return;
    }

    if (auto* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        if (unary->op == "not") {
            genCondition(unary->operand, labelFalse, labelTrue);
            return;
        }

        // Unary negation handled as expression
        std::string cond = genExpression(expr);
        emit("IF " + cond + " THEN " + labelTrue);
        emit("GOTO " + labelFalse);
        return;
    }

    // Fallback (non-binary expression)
    std::string cond = genExpression(expr);
    emit("IF " + cond + " THEN " + labelTrue);
    emit("GOTO " + labelFalse);
}



// ------------------- Utility -------------------

void CodeGen::printCode() const {
    for (const auto& line : code) std::cout << line << std::endl;
}

std::string CodeGen::resolveVariable(const std::string& name) const {
    if (!symbolTable) return name;
    if (symbolTable->isDeclared(name)) return name;
    return name;
}

std::string CodeGen::toString() const {
    std::string codestring;
    for (const auto& line : this->code) {
        codestring += line;
    }
    return codestring;
}

void CodeGen::saveToHTML() const {
    std::stringstream htmlContent;
    htmlContent << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<head><title>Intermediate Generated Code</title></head>\n"
                << "<body>\n"
                << "<h1>Intermediate Generated Code</h1>\n"
                << "<pre><code>\n";

    for (const std::string& line : code) {
        htmlContent << line << "\n";
    }

    htmlContent << "</code></pre>\n" << "</body>\n" << "</html>\n";

    std::ofstream outputFile("ICG.html");
    if (outputFile.is_open()) {
        outputFile << htmlContent.str();
        outputFile.close();
        std::cout << "Intermediate code can be found in ICG.html" << std::endl;
    } else {
        std::cerr << "Error: Unable to open file for writing.\n";
    }
}

// ------------------- Post Processing -------------------

void CodeGen::startPostProcess(){

    // Add line number
    int number = 0;
    for (auto& line : this->code) {
        if(line != ""){
            number += 10;
            line = std::to_string(number) + " " + line;
        }
    }
    // Save Labels
    for (const auto& line : this->code) {
        gatherLabel(line);
    }

    for (auto& line : this->code) {
        changeLabelToLineNumber(line);
    }
    return;
}

void CodeGen::gatherLabel(const std::string line){
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split line into separate strings
    while (iss >> token) {
        tokens.push_back(token);
    }
    // Any line with less than 3 words and numbers doesn't have labels
    if (tokens.size() < 3) {
        return;
    }
    
    // First token should be a number
    int lineNumber;
    try {
        lineNumber = std::stoi(tokens[0]);
    } catch (...) {
        return;
    }
    
    if (tokens[1] != "REM") {
        return;
    }
    
    if (tokens[tokens.size() - 1].find("LBL") != std::string::npos ) {
        // && tokens[tokens.size() - 1].find("WHILE") == std::string::npos
        lineLabelMap[tokens[tokens.size() - 1]] = lineNumber;
        return;
    }
    
}

void CodeGen::changeLabelToLineNumber(std::string &line){
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    // Split line into separate strings with a delimiter of a whitespace
    while (iss >> token) {
        tokens.push_back(token);
    }
    // Any line with less than 3 words and numbers doesn't have labels
    if (tokens.size() < 3) {
        return;
    }
    
    // First token should be a number
    int lineNumberFromToken;
    try {
        lineNumberFromToken = std::stoi(tokens[0]);
    } catch (...) {
        return;
    }

    bool changeLabel = false;
    for (auto& [label, lineNumber] : lineLabelMap){
        if(lineNumber != lineNumberFromToken && label == tokens[tokens.size() - 1]){
            line = "";
            for(size_t i = 0; i < tokens.size() - 1; i++){
                line += tokens[i] + " ";
            };
            line += std::to_string(lineNumber);
            return;
        }
    }
}
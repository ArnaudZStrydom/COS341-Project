#include "codegen.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>
#include <cctype> // Needed for toupper

// =================== PUBLIC ===================

void CodeGen::generate(ProgramNode* program) {
    code.clear();
    tempCounter = 0;
    labelCounter = 0;
    inlineCounter = 0;
    astProgramRoot = program; // Store the root node

    if (!program) return;

    // Generate main program code first
    if (program->main) {
        VarRenameMap emptyMap; // Main has no renames
        genStatementList(program->main->statements, this->code, emptyMap);
    }
}

void CodeGen::saveCode() const {
    std::ofstream outputFile("BASIC_EXECUTABLE.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Could not open BASIC_EXECUTABLE.txt" << std::endl;
        return;
    }
    for (const auto& line : code) {
        outputFile << line << std::endl;
    }
    std::cout << "Executable BASIC code successfully generated in BASIC_EXECUTABLE.txt" << std::endl;
    outputFile.close();
}

/**
 * @brief Implements the inlining logic from COS 341 L16.pdf
 * This is a multi-pass process over the generated intermediate code.
 */
void CodeGen::performInlining() {
    if (!astProgramRoot) return;

    // A map to find the AST node for a function by name
    std::map<std::string, AstNode*> funcAstMap;
    if (astProgramRoot->procs) {
        for (auto* proc : astProgramRoot->procs->elements) {
            funcAstMap[proc->name] = proc;
        }
    }
    if (astProgramRoot->funcs) {
        for (auto* func : astProgramRoot->funcs->elements) {
            funcAstMap[func->name] = func;
        }
    }


    std::vector<std::string> newCode;
    bool codeChanged = true;

    // We loop until no more inlining is possible (handles nested calls)
    while(codeChanged) {
        codeChanged = false;
        newCode.clear();

        // Regex to find CALL commands
        std::regex callRegex(R"((?:(t\d+)\s*=\s*)?CALL_(\w+)\(([^)]*)\))");

        for (const std::string& line : code) {
            std::smatch match;

            if (std::regex_search(line, match, callRegex)) {
                codeChanged = true; // We found a call, so we'll need to loop again

                std::string assignmentVar = match[1].str(); // t_var (empty for PROC)
                std::string funcName = match[2].str();      // funcname

                // 1. Find the function's AST node
                if (funcAstMap.find(funcName) == funcAstMap.end()) {
                    std::cerr << "Error: Definition for " << funcName << " not found. Skipping inlining." << std::endl;
                    newCode.push_back(line); // Keep the bad call
                    continue;
                }
                AstNode* funcNode = funcAstMap[funcName];
                BodyNode* funcBody = nullptr;
                AstNodeList<VarNode>* funcParams = nullptr;

                if (auto* proc = dynamic_cast<ProcDefNode*>(funcNode)) {
                    funcBody = proc->body;
                    funcParams = proc->params;
                } else if (auto* func = dynamic_cast<FuncDefNode*>(funcNode)) {
                    funcBody = func->body;
                    funcParams = func->params;
                }

                if (!funcBody) continue; // Should not happen

                // 2. Create the variable rename map
                VarRenameMap varMap;

                // 3. Generate parameter assignment code
                std::string argsStr = match[3].str();
                std::vector<std::string> callArgs;
                if (!argsStr.empty()) {
                    std::stringstream ss(argsStr);
                    std::string arg;
                    while (std::getline(ss, arg, ',')) {
                        callArgs.push_back(arg);
                    }
                }

                for (size_t i = 0; i < funcParams->elements.size(); ++i) {
                    std::string paramName = funcParams->elements[i]->name;
                    std::string newParamName = newInlinedVar(paramName);
                    varMap[paramName] = newParamName;

                    if (i < callArgs.size()) {
                        // Use BASIC-compatible assignment
                        newCode.push_back(newParamName + " = " + callArgs[i]);
                    }
                }

                // 4. Map the function's local variables
                for(auto* local : funcBody->locals->elements) {
                    varMap[local->name] = newInlinedVar(local->name);
                }

                // 5. Generate the inlined body
                // We pass the map, and the assignmentVar (e.g., "t9")
                // which will be used to replace the "return" statement.
                genStatementList(funcBody->statements, newCode, varMap, assignmentVar);

            } else {
                // Not a CALL line, just copy it over
                newCode.push_back(line);
            }
        }
        code = newCode; // The new code becomes the code for the next pass
    }
}

// =================== PRIVATE ===================

std::string CodeGen::newTemp() {
    ++tempCounter;
    return "t" + std::to_string(tempCounter);
}

// *** CORRECTED FUNCTION ***
std::string CodeGen::newInlinedVar(const std::string& varName) {
    ++inlineCounter;
    if (varName.empty()) {
        return "V" + std::to_string(inlineCounter); // Fallback
    }
    // Use first letter (uppercased) + counter for BASIC compatibility
    char firstChar = std::toupper(varName[0]);
    return std::string(1, firstChar) + std::to_string(inlineCounter);
}


std::string CodeGen::newLabel(const std::string& prefix) {
    ++labelCounter;
    return prefix + "_" + std::to_string(labelCounter);
}

void CodeGen::emit(const std::string& line) {
    code.push_back(line);
}

// Helper overload to emit into a specific vector (for inlining)
void CodeGen::emit(const std::string& line, std::vector<std::string>& codeBlock) {
    codeBlock.push_back(line);
}

// ------------------- Program & Statements -------------------

// This function is no longer used, as 'generate' handles main directly
std::string CodeGen::genProgram(ProgramNode* program) {
    return ""; // Obsolete
}

void CodeGen::genStatementList(AstNodeList<StatementNode>* stmts, std::vector<std::string>& codeBlock, VarRenameMap& varMap, const std::string& funcReturnVar) {
    if (!stmts) return;
    for (auto* stmt : stmts->elements) {
        genStatement(stmt, codeBlock, varMap, funcReturnVar);
    }
}

void CodeGen::genStatement(StatementNode* stmt, std::vector<std::string>& codeBlock, VarRenameMap& varMap, const std::string& funcReturnVar) {
    if (!stmt) return;

    if (auto* halt = dynamic_cast<HaltNode*>(stmt)) {
        emit("STOP", codeBlock);
    }
    else if (auto* print = dynamic_cast<PrintNode*>(stmt)) {
        std::string e = genExpression(print->expression, codeBlock, varMap);
        emit("PRINT " + e, codeBlock);
    }
    else if (auto* assign = dynamic_cast<AssignNode*>(stmt)) {
        std::string varName = resolveVariable(assign->var->name, varMap);
        std::string rhs = genExpression(assign->expression, codeBlock, varMap);
        // Use BASIC-compatible assignment (no LET)
        emit(varName + " = " + rhs, codeBlock);
    }
    else if (auto* procCall = dynamic_cast<ProcCallNode*>(stmt)) {
        std::string params = "";
        if (procCall->args) {
            for (auto* a : procCall->args->elements) params += genExpression(a, codeBlock, varMap) + ",";
            if (!params.empty()) params.pop_back();
        }
        // This CALL_ line will be replaced by inlining
        emit("CALL_" + procCall->name + "(" + params + ")", codeBlock);
    }
    else if (auto* ifNode = dynamic_cast<IfNode*>(stmt)) {
        std::string labelThen = newLabel("LBL_THEN");
        std::string labelExit = newLabel("LBL_EXIT");
        genCondition(ifNode->condition, codeBlock, varMap, labelThen, labelExit);
        emit("REM " + labelThen, codeBlock);
        genStatementList(ifNode->then_branch, codeBlock, varMap, funcReturnVar);
        emit("REM " + labelExit, codeBlock);
    }
    else if (auto* ifElseNode = dynamic_cast<IfElseNode*>(stmt)) {
        std::string labelThen = newLabel("LBL_THEN");
        std::string labelElse = newLabel("LBL_ELSE");
        std::string labelExit = newLabel("LBL_EXIT");

        genCondition(ifElseNode->condition, codeBlock, varMap, labelThen, labelElse);

        // Else branch
        emit("REM " + labelElse, codeBlock);
        genStatementList(ifElseNode->else_branch, codeBlock, varMap, funcReturnVar);
        emit("GOTO " + labelExit, codeBlock);

        // Then branch
        emit("REM " + labelThen, codeBlock);
        genStatementList(ifElseNode->then_branch, codeBlock, varMap, funcReturnVar);

        // Exit
        emit("REM " + labelExit, codeBlock);
    }
    else if (auto* whileNode = dynamic_cast<WhileNode*>(stmt)) {
        std::string labelStart = newLabel("LBL_WHILE");
        std::string labelExit = newLabel("LBL_EXIT_WHILE");

        emit("REM " + labelStart, codeBlock);
        genCondition(whileNode->condition, codeBlock, varMap, labelStart + "_BODY", labelExit);

        emit("REM " + labelStart + "_BODY", codeBlock);
        genStatementList(whileNode->body, codeBlock, varMap, funcReturnVar);
        emit("GOTO " + labelStart, codeBlock);

        emit("REM " + labelExit, codeBlock);
    }
    else if (auto* doUntilNode = dynamic_cast<DoUntilNode*>(stmt)) {
        std::string labelStart = newLabel("LBL_DO");
        std::string labelExit = newLabel("LBL_EXIT_DO");

        emit("REM " + labelStart, codeBlock);
        genStatementList(doUntilNode->body, codeBlock, varMap, funcReturnVar);
        // Do-until: jump if condition is false (evaluate condition, jump to start if false)
        // We need to generate the condition check *before* the jump
        // BASIC doesn't have a direct IF NOT THEN GOTO, so we use IF THEN GOTO exit
        genCondition(doUntilNode->condition, codeBlock, varMap, labelExit, labelStart);
        emit("REM " + labelExit, codeBlock);
    }
    else if (auto* returnNode = dynamic_cast<ReturnNode*>(stmt)) {
        // This is the "return EXPRESSION" line from L16.pdf
        // Replace it with "t_i = EXPRESSION"
        // funcReturnVar is the "t_i" (e.g., "t9")
        if (!funcReturnVar.empty()) {
            std::string e = genAtom(returnNode->expression, varMap);
             // Use BASIC-compatible assignment (no LET)
            emit(funcReturnVar + " = " + e, codeBlock);
        }
    }
}


// ------------------- Expressions -------------------

std::string CodeGen::genAtom(ExpressionNode* atom, VarRenameMap& varMap) {
    if (auto* var = dynamic_cast<VarNode*>(atom)) {
        return resolveVariable(var->name, varMap);
    }
    if (auto* number = dynamic_cast<NumberNode*>(atom)) {
        return number->value;
    }
    return ""; // Should not happen based on grammar
}

std::string CodeGen::genExpression(ExpressionNode* expr, std::vector<std::string>& codeBlock, VarRenameMap& varMap, bool inCondition) {
    if (!expr) return "";

    if (auto* number = dynamic_cast<NumberNode*>(expr)) {
        return number->value; // constants stay inline
    }

    if (auto* var = dynamic_cast<VarNode*>(expr)) {
        return resolveVariable(var->name, varMap); // variables stay inline
    }

    if (auto* stringNode = dynamic_cast<StringNode*>(expr)) {
        return "\"" + stringNode->value + "\"";
    }

    if (auto* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        std::string operand = genExpression(unary->operand, codeBlock, varMap);
        std::string tmp = newTemp();

        // BASIC uses operators differently
        if (unary->op == "neg") emit(tmp + " = -" + operand, codeBlock);
        // BASIC doesn't have a direct '!', we handle 'not' in genCondition
        else if (unary->op == "not") {
             // For safety, generate a temporary boolean representation if needed outside condition
            std::string zero = newTemp();
            emit(zero + " = 0", codeBlock);
            emit(tmp + " = (" + operand + " = " + zero + ")", codeBlock); // tmp = -1 if operand is 0, else 0
        }
        else emit(tmp + " = " + unary->op + " " + operand, codeBlock); // Should not happen?

        return tmp;
    }

    if (auto* binary = dynamic_cast<BinaryOpNode*>(expr)) {
        std::string op;
        bool isComparison = false;

        if (binary->op == "plus") op = " + ";
        else if (binary->op == "minus") op = " - ";
        else if (binary->op == "mult") op = " * ";
        else if (binary->op == "div") op = " / ";
        // BASIC uses = for comparison too
        else if (binary->op == "eq") { op = " = "; isComparison = true; }
         // BASIC uses <> for not equal
        else if (binary->op == "ne") { op = " <> "; isComparison = true; }
        else if (binary->op == "gt" || binary->op == ">") { op = " > "; isComparison = true; }
        else if (binary->op == "lt" || binary->op == "<") { op = " < "; isComparison = true; }
        else if (binary->op == "ge") { op = " >= "; isComparison = true; }
        else if (binary->op == "le") { op = " <= "; isComparison = true; }
        // BASIC uses AND/OR operators
        else if (binary->op == "and") { op = " AND "; isComparison = true; }
        else if (binary->op == "or") { op = " OR "; isComparison = true; }
        else op = " " + binary->op + " "; // Should not happen

        std::string left = genExpression(binary->left, codeBlock, varMap);
        std::string right = genExpression(binary->right, codeBlock, varMap);

        // always emit temporaries for both operands
        std::string tmpLeft = newTemp();
        emit(tmpLeft + " = " + left, codeBlock);

        std::string tmpRight = newTemp();
        emit(tmpRight + " = " + right, codeBlock);

        // comparisons still get a temporary to hold boolean result
        // BASIC evaluates boolean expressions to -1 (true) or 0 (false)
        std::string tmp = newTemp();
        emit(tmp + " = (" + tmpLeft + op + tmpRight + ")", codeBlock);

        return tmp;
    }

    if (auto* funcCall = dynamic_cast<FuncCallNode*>(expr)) {
        std::string params = "";
        if (funcCall->args) {
            for (auto* a : funcCall->args->elements)
                params += genExpression(a, codeBlock, varMap) + ",";
            if (!params.empty()) params.pop_back();
        }
        std::string tmp = newTemp();
        // This t_i = CALL_... line will be replaced by inlining
        emit(tmp + " = CALL_" + funcCall->name + "(" + params + ")", codeBlock);
        return tmp;
    }

    return "";
}


// ------------------- Conditional Flattening -------------------
void CodeGen::genCondition(ExpressionNode* expr, std::vector<std::string>& codeBlock, VarRenameMap& varMap,
                           const std::string& labelTrue,
                           const std::string& labelFalse) {
    if (!expr) return;

    if (auto* binary = dynamic_cast<BinaryOpNode*>(expr)) {
        std::string op;
        bool useExpressionResult = false; // Flag for AND/OR

        if (binary->op == "eq") op = " = ";
        else if (binary->op == "ne") op = " <> ";
        else if (binary->op == "gt" || binary->op == ">") op = " > ";
        else if (binary->op == "lt" || binary->op == "<") op = " < ";
        else if (binary->op == "ge") op = " >= ";
        else if (binary->op == "le") op = " <= ";
        // For AND/OR, we evaluate the whole expression first
        else if (binary->op == "and") { useExpressionResult = true; }
        else if (binary->op == "or") { useExpressionResult = true; }
        else { // Should not happen for conditions
             emit("# ERROR: Invalid operator in condition: " + binary->op, codeBlock);
             return;
        }

        if (useExpressionResult) {
            // Evaluate the whole AND/OR expression into a temporary
             std::string condResult = genExpression(expr, codeBlock, varMap);
             // BASIC uses -1 for true, 0 for false. Check if result is not 0.
             std::string zero = newTemp();
             emit(zero + " = 0", codeBlock);
             emit("IF " + condResult + " <> " + zero + " THEN " + labelTrue, codeBlock);
        } else {
             // Handle standard comparisons
            std::string left = genExpression(binary->left, codeBlock, varMap);
            std::string right = genExpression(binary->right, codeBlock, varMap);

            std::string tmpLeft = newTemp();
            emit(tmpLeft + " = " + left, codeBlock);

            std::string tmpRight = newTemp();
            emit(tmpRight + " = " + right, codeBlock);

            emit("IF " + tmpLeft + op + tmpRight + " THEN " + labelTrue, codeBlock);
        }

        emit("GOTO " + labelFalse, codeBlock);
        return;
    }

    if (auto* unary = dynamic_cast<UnaryOpNode*>(expr)) {
        // Handle "not" by swapping labels
        if (unary->op == "not") {
            genCondition(unary->operand, codeBlock, varMap, labelFalse, labelTrue);
            return;
        }
    }

    // Fallback (e.g., a single variable or number as a condition)
    // Check if the value is non-zero (true in BASIC)
    std::string cond = genExpression(expr, codeBlock, varMap);
    std::string zero = newTemp();
    emit(zero + " = 0", codeBlock);
    emit("IF " + cond + " <> " + zero + " THEN " + labelTrue, codeBlock);
    emit("GOTO " + labelFalse, codeBlock);
}



// ------------------- Utility -------------------

void CodeGen::printCode() const {
    for (const auto& line : code) std::cout << line << std::endl;
}

std::string CodeGen::resolveVariable(const std::string& name, VarRenameMap& varMap) {
    if (varMap.count(name)) {
        return varMap.at(name);
    }
    // Not a mapped local/param, so it must be global or main local
    // Check if it's a temporary variable (starts with 't' followed by digits)
    if (name.length() > 1 && name[0] == 't' && std::all_of(name.begin() + 1, name.end(), ::isdigit)) {
        return name;
    }
    // Assume it's a global or main local if not found in map and not a temp
    // BASIC is often case-insensitive, convert to upper for safety? Or assume case-sensitive?
    // Let's assume case-sensitive for now as per SPL spec.
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
                << "<head><title>Generated BASIC Code</title></head>\n" // Changed title
                << "<body>\n"
                << "<h1>Generated BASIC Code</h1>\n"
                << "<pre><code>\n";

    for (const std::string& line : code) {
        htmlContent << line << "\n";
    }

    htmlContent << "</code></pre>\n" << "</body>\n" << "</html>\n";

    std::ofstream outputFile("ICG.html");
    if (outputFile.is_open()) {
        outputFile << htmlContent.str();
        outputFile.close();
        // Updated message
        std::cout << "HTML preview of generated code saved to ICG.html" << std::endl;
    } else {
        std::cerr << "Error: Unable to open file for writing.\n";
    }
}

// ------------------- Post Processing -------------------

void CodeGen::startPostProcess(){
    // Add line number
    int number = 0;
    std::vector<std::string> numberedCode;
    for (auto& line : this->code) {
        if(!line.empty()){
            number += 10;
            numberedCode.push_back(std::to_string(number) + " " + line);
        }
    }
    this->code = numberedCode;

    // Save Labels
    lineLabelMap.clear();
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

    if (tokens.size() < 2) { // Need at least line number and command
        return;
    }

    // First token should be a number
    int lineNumberFromToken;
    try {
        lineNumberFromToken = std::stoi(tokens[0]);
    } catch (...) {
        return;
    }

    // Check the last token (for GOTO) or third-to-last (for IF THEN)
    std::string targetLabel = "";
    size_t labelPos = std::string::npos;

    // Check for GOTO Label
    // Ensure there are enough tokens before checking indices
    if (tokens.size() >= 3 && tokens[tokens.size() - 2] == "GOTO") {
        targetLabel = tokens[tokens.size() - 1];
        labelPos = tokens.size() - 1;
    }
    // Check for IF ... THEN Label
    // Ensure there are enough tokens before checking indices
    else if (tokens.size() >= 5 && tokens[tokens.size() - 2] == "THEN") {
         targetLabel = tokens[tokens.size() - 1];
         labelPos = tokens.size() - 1;
    }

    if (!targetLabel.empty() && lineLabelMap.count(targetLabel)) {
        // Only modify if it's a jump (don't modify the REM line itself)
        // Check if the command is REM
         if (tokens[1] != "REM") {
             line = "";
            for(size_t i = 0; i < labelPos; i++){
                line += tokens[i] + " ";
            };
            line += std::to_string(lineLabelMap.at(targetLabel));
            // Add remaining parts of the line if any (shouldn't be for GOTO/THEN)
            for (size_t i = labelPos + 1; i < tokens.size(); ++i) {
                line += " " + tokens[i];
            }
            return;
        }
    }
}


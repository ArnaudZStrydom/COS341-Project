// main.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include "spl.tab.hpp"
#include "ast.h" // Make sure to include your AST header
#include "type_checker.h"
#include "Intermediate-Code-Generation/codegen.h"

extern void initialize_lexer(const std::string& source);
extern int yyparse();
extern AstNode* ast_root; // The global pointer from spl.y

std::string readFileToString(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file.txt>" << std::endl;
        return 1;
    }

    std::string source_code;
    try {
        source_code = readFileToString(argv[1]);
        initialize_lexer(source_code);
        yyparse();

        // Parse successful, now perform type checking
        if (ast_root) {
            std::cout << "\n--- Abstract Syntax Tree ---" << std::endl;
            ast_root->print();
            
            // Perform type checking
            std::cout << "\n--- Type Checking ---" << std::endl;
            TypeChecker typeChecker;
            bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
            
            // Print type checker results
            typeChecker.printErrors();
            
            if (typeCheckPassed) {
                typeChecker.printSymbolTable();
                std::cout << "Type checking passed successfully!" << std::endl;
            } else {
                std::cout << "Type checking failed!" << std::endl;
                return 1; // Exit with error code
            }

            //Code Generation
            std::cout << "\n--- BASIC Code Generation ---" << std::endl;
            CodeGen codeGen;
            codeGen.setSymbolTable(&typeChecker.getSymbolTable());

            // 1. Generate intermediate code with PROC/FUNC/CALL
            std::cout << "Step 1: Generating intermediate code..." << std::endl;
            codeGen.generate(static_cast<ProgramNode*>(ast_root));

            // *** MOVED: Save HTML with NON-EXECUTABLE Intermediate Code (as per Project Type B spec) ***
            std::cout << "Saving intermediate code preview to ICG.html..." << std::endl;
            codeGen.saveToHTML(); 
            
            // 2. Perform inlining (Project Type A)
            // This replaces all CALL/PROC/FUNC with inlined BASIC
            std::cout << "Step 2: Performing inlining..." << std::endl;
            codeGen.performInlining();

            // 3. Post-process to add line numbers and fix GOTO/THEN labels
            std::cout << "Step 3: Post-processing to executable BASIC..." << std::endl;
            codeGen.startPostProcess();
            
            // 4. Save final executable code
            std::cout << "Step 4: Saving final executable code..." << std::endl;
            codeGen.saveCode(); // Saves to ICG.txt
            std::cout << "Executable BASIC code successfully generated in ICG.txt" << std::endl;

            delete ast_root; // Clean up the memory for the entire tree
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
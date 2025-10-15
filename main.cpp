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
                std::cout << "Type checking passed successfully!" << std::endl;
            } else {
                std::cout << "Type checking failed!" << std::endl;
                return 1; // Exit with error code
            }

        //Intermediate Code Generation
        std::cout << "\n--- Intermediate Code Generation ---" << std::endl;
        CodeGen codeGen;
        codeGen.generate(static_cast<ProgramNode*>(ast_root));
        std::cout<<"Code can be found in ICG.txt"<<std::endl;
        //codeGen.printCode();
        codeGen.saveCode();

            delete ast_root; // Clean up the memory for the entire tree
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
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
        
        int parse_res = yyparse();
        if(parse_res ==  0){
            std::cout<<"Syntax accepted"<<std::endl;
        }else{
            return 1;
        }

        if (ast_root) {
            std::cout<<"Tokens accepted"<<std::endl;
            //ast_root->print();

            ast_root->checkNames();
            std::cout<<"Variable Naming and Function Naming accepted"<<std::endl;
            
            TypeChecker typeChecker;
            bool typeCheckPassed = typeChecker.typeCheck(static_cast<ProgramNode*>(ast_root));
            
            typeChecker.printErrors();
            
            if (typeCheckPassed) {
                //typeChecker.printSymbolTable();
                std::cout << "Types accepted" << std::endl;
            } else {
                std::cout << "Type error:" << std::endl;
                return 1; // Exit with error code
            }

            //Code Generation
            CodeGen codeGen;
            codeGen.setSymbolTable(&typeChecker.getSymbolTable());

            codeGen.generate(static_cast<ProgramNode*>(ast_root));

            codeGen.saveToHTML(); 
            
            codeGen.performInlining();

            codeGen.startPostProcess();
            
            codeGen.saveCode(); 

            delete ast_root; 
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Lexical error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
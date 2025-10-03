#include "spl_lexer.h"
#include "spl.tab.hpp"
#include <memory>
#include <iostream>

static std::unique_ptr<Lexer> lexer_instance;
int current_line_number = 1;

int yylex() {
    Token token = lexer_instance->getNextToken();
    current_line_number = lexer_instance->line_number_;

    if (token.type == 0) {
        return 0;
    }

    yylval.sval = new std::string(token.value);
    return token.type;
}

void initialize_lexer(const std::string& source) {
    lexer_instance = std::make_unique<Lexer>(source);
}
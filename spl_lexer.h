#ifndef SPL_LEXER_H
#define SPL_LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "spl.tab.hpp" // <-- IMPORTANT: Include Bison's generated header

// The old 'enum class TokenType' is completely removed.

// The Token struct now uses Bison's 'yytokentype'
struct Token {
    yytokentype type;
    std::string value;
    void print() const;
};

class Lexer {
public:
    Lexer(const std::string& source);
    Token getNextToken();
    int line_number_ = 1;

private:
    char peek();
    void advance();
    void skipWhitespace();
    Token identifier();
    Token number();
    Token stringLiteral();

    std::string source_;
    size_t current_pos_;
    // The map now uses Bison's 'yytokentype'
    std::unordered_map<std::string, yytokentype> keywords_;
};

#endif // SPL_LEXER_H
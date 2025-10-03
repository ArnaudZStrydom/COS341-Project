#include "spl_lexer.h"
#include <iostream>
#include <cctype>
#include <stdexcept>

// This function is for debugging and now takes a yytokentype
std::string tokenTypeToString(yytokentype type) {
    switch (type) {
        case GLOB:       return "GLOB";
        case PROC:       return "PROC";
        case FUNC:       return "FUNC";
        case MAIN:       return "MAIN";
        case LOCAL:      return "LOCAL";
        case VAR:        return "VAR";
        case RETURN:     return "RETURN";
        case HALT:       return "HALT";
        case PRINT:      return "PRINT";
        case WHILE:      return "WHILE";
        case DO:         return "DO";
        case UNTIL:      return "UNTIL";
        case IF:         return "IF";
        case ELSE:       return "ELSE";
        case NEG:        return "NEG";
        case NOT:        return "NOT";
        case EQ:         return "EQ";
        case OR:         return "OR";
        case AND:        return "AND";
        case PLUS:       return "PLUS";
        case MINUS:      return "MINUS";
        case MULT:       return "MULT";
        case DIV:        return "DIV";
        case LBRACE:     return "LBRACE";
        case RBRACE:     return "RBRACE";
        case LPAREN:     return "LPAREN";
        case RPAREN:     return "RPAREN";
        case SEMICOLON:  return "SEMICOLON";
        case ASSIGN:     return "ASSIGN";
        case GT:         return "GT";
        case IDENTIFIER: return "IDENTIFIER";
        case NUMBER:     return "NUMBER";
        case STRING:     return "STRING";
        default:         return "INVALID_TOKEN";
    }
}

void Token::print() const {
    std::cout << "Type: " << tokenTypeToString(type)
              << ", Value: '" << value << "'" << std::endl;
}

Lexer::Lexer(const std::string& source)
    : source_(source), current_pos_(0) {
    keywords_["glob"] = GLOB;
    keywords_["proc"] = PROC;
    keywords_["func"] = FUNC;
    keywords_["main"] = MAIN;
    keywords_["local"] = LOCAL;
    keywords_["var"] = VAR;
    keywords_["return"] = RETURN;
    keywords_["halt"] = HALT;
    keywords_["print"] = PRINT;
    keywords_["while"] = WHILE;
    keywords_["do"] = DO;
    keywords_["until"] = UNTIL;
    keywords_["if"] = IF;
    keywords_["else"] = ELSE;
    keywords_["neg"] = NEG;
    keywords_["not"] = NOT;
    keywords_["eq"] = EQ;
    keywords_["or"] = OR;
    keywords_["and"] = AND;
    keywords_["plus"] = PLUS;
    keywords_["minus"] = MINUS;
    keywords_["mult"] = MULT;
    keywords_["div"] = DIV;
}

Token Lexer::getNextToken() {
    skipWhitespace();

    if (current_pos_ >= source_.length()) {
        return {yytokentype(0), ""};
    }

    char current_char = peek();

    // Use islower to match the spec for user-defined names
    if (islower(current_char)) {
        return identifier();
    }

    if (isdigit(current_char)) {
        return number();
    }

    if (current_char == '"') {
        return stringLiteral();
    }

    advance();
    switch (current_char) {
        case '{': return {LBRACE, "{"};
        case '}': return {RBRACE, "}"};
        case '(': return {LPAREN, "("};
        case ')': return {RPAREN, ")"};
        case ';': return {SEMICOLON, ";"};
        case '=': return {ASSIGN, "="};
        case '>': return {GT, ">"};
        default:  throw std::runtime_error("Unrecognized character: " + std::string(1, current_char) + ". Line: " + std::to_string(line_number_));
    }
}

char Lexer::peek() {
    if (current_pos_ >= source_.length()) {
        return '\0';
    }
    return source_[current_pos_];
}

void Lexer::advance() {
    current_pos_++;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        if (peek() == '\n') {
            line_number_++;
        }
        advance();
    }
}

Token Lexer::identifier() {
    size_t start_pos = current_pos_;
    while (islower(peek())) {
        advance();
    }
    while (isdigit(peek())) {
        advance();
    }
    std::string value = source_.substr(start_pos, current_pos_ - start_pos);
    auto it = keywords_.find(value);
    if (it != keywords_.end()) {
        return {it->second, value};
    }
    return {IDENTIFIER, value};
}

Token Lexer::number() {
    size_t start_pos = current_pos_;
    char first_char = peek();
    if (first_char == '0') {
        advance();
        if (isdigit(peek())) {
            throw std::runtime_error("Invalid number format: leading zero on multi-digit number. Line: " + std::to_string(line_number_));
        }
        return {NUMBER, "0"};
    } else {
        while (isdigit(peek())) {
            advance();
        }
        std::string value = source_.substr(start_pos, current_pos_ - start_pos);
        return {NUMBER, value};
    }
}

Token Lexer::stringLiteral() {
    advance(); // Skip opening quote
    size_t start_pos = current_pos_;
    while (isalnum(peek())) { 
        advance();
    }
    std::string value = source_.substr(start_pos, current_pos_ - start_pos);
    if (peek() != '"') {
        throw std::runtime_error("Unterminated or invalid string literal. Only letters and digits are allowed. Line: " + std::to_string(line_number_));
    }
    advance(); 
    if (value.length() > 15) {
        throw std::runtime_error("String literal exceeds maximum length of 15 characters. Line: " + std::to_string(line_number_));
    }
    return {STRING, value};
}
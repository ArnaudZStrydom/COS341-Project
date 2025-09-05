#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <stdexcept>
#include <fstream>
#include <sstream>

enum class TokenType {
    GLOB, PROC, FUNC, MAIN, LOCAL, VAR, RETURN, HALT, PRINT,
    WHILE, DO, UNTIL, IF, ELSE, NEG, NOT, EQ, OR, AND,
    PLUS, MINUS, MULT, DIV,

    // Symbols
    LBRACE,     // {
    RBRACE,     // }
    LPAREN,     // (
    RPAREN,     // )
    SEMICOLON,  // ;
    ASSIGN,     // =
    GT,         // >

    // Literals & Identifiers
    IDENTIFIER,
    NUMBER,
    STRING,

    // Control
    END_OF_FILE,
    UNKNOWN
};

// Helper function to convert a TokenType enum to its string representation
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        // Keywords
        case TokenType::GLOB:       return "GLOB";
        case TokenType::PROC:       return "PROC";
        case TokenType::FUNC:       return "FUNC";
        case TokenType::MAIN:       return "MAIN";
        case TokenType::LOCAL:      return "LOCAL";
        case TokenType::VAR:        return "VAR";
        case TokenType::RETURN:     return "RETURN";
        case TokenType::HALT:       return "HALT";
        case TokenType::PRINT:      return "PRINT";
        case TokenType::WHILE:      return "WHILE";
        case TokenType::DO:         return "DO";
        case TokenType::UNTIL:      return "UNTIL";
        case TokenType::IF:         return "IF";
        case TokenType::ELSE:       return "ELSE";
        case TokenType::NEG:        return "NEG";
        case TokenType::NOT:        return "NOT";
        case TokenType::EQ:         return "EQ";
        case TokenType::OR:         return "OR";
        case TokenType::AND:        return "AND";
        case TokenType::PLUS:       return "PLUS";
        case TokenType::MINUS:      return "MINUS";
        case TokenType::MULT:       return "MULT";
        case TokenType::DIV:        return "DIV";

        // Symbols
        case TokenType::LBRACE:     return "LBRACE";
        case TokenType::RBRACE:     return "RBRACE";
        case TokenType::LPAREN:     return "LPAREN";
        case TokenType::RPAREN:     return "RPAREN";
        case TokenType::SEMICOLON:  return "SEMICOLON";
        case TokenType::ASSIGN:     return "ASSIGN";
        case TokenType::GT:         return "GT";

        // Literals & Identifiers
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER:     return "NUMBER";
        case TokenType::STRING:     return "STRING";

        // Control
        case TokenType::END_OF_FILE:return "END_OF_FILE";
        case TokenType::UNKNOWN:    return "UNKNOWN";
        default:                    return "INVALID_TOKEN";
    }
}

// Represents a single token with its type and value
struct Token {
    TokenType type;
    std::string value;
    void print() const {
        std::cout << "Type: " << tokenTypeToString(type)
                  << ", Value: '" << value << "'" << std::endl;
    }
};

// The Lexer class, responsible for turning source code into tokens
class Lexer {
public:
    Lexer(const std::string& source)
        : source_(source), current_pos_(0) {
        keywords_["glob"] = TokenType::GLOB;
        keywords_["proc"] = TokenType::PROC;
        keywords_["func"] = TokenType::FUNC;
        keywords_["main"] = TokenType::MAIN;
        keywords_["local"] = TokenType::LOCAL;
        keywords_["var"] = TokenType::VAR;
        keywords_["return"] = TokenType::RETURN;
        keywords_["halt"] = TokenType::HALT;
        keywords_["print"] = TokenType::PRINT;
        keywords_["while"] = TokenType::WHILE;
        keywords_["do"] = TokenType::DO;
        keywords_["until"] = TokenType::UNTIL;
        keywords_["if"] = TokenType::IF;
        keywords_["else"] = TokenType::ELSE;
        keywords_["neg"] = TokenType::NEG;
        keywords_["not"] = TokenType::NOT;
        keywords_["eq"] = TokenType::EQ;
        keywords_["or"] = TokenType::OR;
        keywords_["and"] = TokenType::AND;
        keywords_["plus"] = TokenType::PLUS;
        keywords_["minus"] = TokenType::MINUS;
        keywords_["mult"] = TokenType::MULT;
        keywords_["div"] = TokenType::DIV;
    }

    Token getNextToken() {
        skipWhitespace();

        if (current_pos_ >= source_.length()) {
            return {TokenType::END_OF_FILE, ""};
        }

        char current_char = peek();

        if (isalpha(current_char)) {
            return identifier();
        }

        if (isdigit(current_char)) {
            return number();
        }

        if (current_char == '"') {
            return stringLiteral();
        }

        advance(); // Consume the character
        switch (current_char) {
            case '{': return {TokenType::LBRACE, "{"};
            case '}': return {TokenType::RBRACE, "}"};
            case '(': return {TokenType::LPAREN, "("};
            case ')': return {TokenType::RPAREN, ")"};
            case ';': return {TokenType::SEMICOLON, ";"};
            case '=': return {TokenType::ASSIGN, "="};
            case '>': return {TokenType::GT, ">"};
            default:  throw std::runtime_error("Unrecognized character: " + std::string(1, current_char));
        }
    }

private:
    std::string source_;
    size_t current_pos_;
    std::unordered_map<std::string, TokenType> keywords_;

    char peek() {
        if (current_pos_ >= source_.length()) {
            return '\0';
        }
        return source_[current_pos_];
    }

    void advance() {
        current_pos_++;
    }

    void skipWhitespace() {
        while (isspace(peek())) {
            advance();
        }
    }

    Token identifier() {
        size_t start_pos = current_pos_;
        if (isalpha(peek())) {
            advance();
        }
        while (isalpha(peek())) {
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
        return {TokenType::IDENTIFIER, value};
    }

    Token number() {
        size_t start_pos = current_pos_;
        while (isdigit(peek())) {
            advance();
        }
        std::string value = source_.substr(start_pos, current_pos_ - start_pos);
        return {TokenType::NUMBER, value};
    }

    Token stringLiteral() {
        advance(); // Consume the opening quote "
        size_t start_pos = current_pos_;
        while (peek() != '"' && peek() != '\0') {
            advance();
        }
        if (peek() == '\0') {
            throw std::runtime_error("Unterminated string literal.");
        }
        std::string value = source_.substr(start_pos, current_pos_ - start_pos);
        if (peek() == '"') {
            advance(); // Consume the closing quote "
        }
        if (value.length() > 15) {
            throw std::runtime_error("String literal exceeds maximum length of 15 characters.");
        }
        return {TokenType::STRING, value};
    }
};

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

    std::string filePath = argv[1];
    std::string source_code;

    try {
        source_code = readFileToString(filePath);

        Lexer lexer(source_code);
        std::cout << "--- Tokenizing " << filePath << " ---" << std::endl;
        Token token = lexer.getNextToken();
        while (token.type != TokenType::END_OF_FILE) {
            token.print();
            token = lexer.getNextToken();
        }
        std::cout << "--- End of File ---" << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1; // Exit with an error code
    }

    return 0;
}
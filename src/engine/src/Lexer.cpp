#include "matforge/engine/Lexer.hpp"
#include <cctype>
#include <cstdlib>

namespace matforge::engine {

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

bool Lexer::isAtEnd() const {
    return cursor_ >= source_.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[cursor_];
}

char Lexer::peekNext() const {
    if (cursor_ + 1 >= source_.size()) return '\0';
    return source_[cursor_ + 1];
}

char Lexer::advance() {
    char ch = source_[cursor_++];
    if (ch == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return ch;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char ch = peek();
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            advance();
        } else if (ch == '%') {
            // Comment until end of line
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::readNumber() {
    std::size_t startCol = column_;
    std::size_t startLine = line_;
    std::size_t startPos = cursor_;

    while (!isAtEnd() && (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.')) {
        if (peek() == '.' && peekNext() == '*') break; // Don't consume .*
        if (peek() == '.' && peekNext() == '/') break; // Don't consume ./
        if (peek() == '.' && peekNext() == '^') break; // Don't consume .^
        advance();
    }

    // Exponent e or E
    if (!isAtEnd() && (peek() == 'e' || peek() == 'E')) {
        advance();
        if (!isAtEnd() && (peek() == '+' || peek() == '-')) {
            advance();
        }
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string text = source_.substr(startPos, cursor_ - startPos);
    double val = std::strtod(text.c_str(), nullptr);
    return Token{TokenType::Number, text, val, startLine, startCol};
}

Token Lexer::readIdentifier() {
    std::size_t startCol = column_;
    std::size_t startLine = line_;
    std::size_t startPos = cursor_;

    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }

    std::string text = source_.substr(startPos, cursor_ - startPos);
    return Token{TokenType::Identifier, text, 0.0, startLine, startCol};
}

Token Lexer::readString(char quote) {
    std::size_t startCol = column_;
    std::size_t startLine = line_;
    advance(); // Consume opening quote

    std::string text;
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\' && peekNext() != '\0') {
            advance(); // Escape
        }
        text += advance();
    }

    if (!isAtEnd() && peek() == quote) {
        advance(); // Consume closing quote
    }

    return Token{TokenType::String, text, 0.0, startLine, startCol};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;

        std::size_t startCol = column_;
        std::size_t startLine = line_;
        char ch = peek();

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            tokens.push_back(readNumber());
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
            tokens.push_back(readIdentifier());
            continue;
        }

        if (ch == '\'' || ch == '"') {
            // In MATLAB, single quote can be transpose if preceded by number, identifier, ')', or ']'
            bool isTranspose = false;
            if (ch == '\'' && !tokens.empty()) {
                TokenType prev = tokens.back().type;
                if (prev == TokenType::Identifier || prev == TokenType::Number ||
                    prev == TokenType::RParen || prev == TokenType::RBracket ||
                    prev == TokenType::Transpose) {
                    isTranspose = true;
                }
            }

            if (isTranspose) {
                advance();
                tokens.push_back(Token{TokenType::Transpose, "'", 0.0, startLine, startCol});
                continue;
            }

            tokens.push_back(readString(ch));
            continue;
        }

        // Two-character operators
        if (ch == '.' && peekNext() == '*') {
            advance(); advance();
            tokens.push_back(Token{TokenType::DotStar, ".*", 0.0, startLine, startCol});
            continue;
        }
        if (ch == '.' && peekNext() == '/') {
            advance(); advance();
            tokens.push_back(Token{TokenType::DotSlash, "./", 0.0, startLine, startCol});
            continue;
        }
        if (ch == '.' && peekNext() == '^') {
            advance(); advance();
            tokens.push_back(Token{TokenType::DotCaret, ".^", 0.0, startLine, startCol});
            continue;
        }
        if (ch == '=' && peekNext() == '=') {
            advance(); advance();
            tokens.push_back(Token{TokenType::Equals, "==", 0.0, startLine, startCol});
            continue;
        }
        if (ch == '~' && peekNext() == '=') {
            advance(); advance();
            tokens.push_back(Token{TokenType::NotEquals, "~=", 0.0, startLine, startCol});
            continue;
        }
        if (ch == '<' && peekNext() == '=') {
            advance(); advance();
            tokens.push_back(Token{TokenType::LessEquals, "<=", 0.0, startLine, startCol});
            continue;
        }
        if (ch == '>' && peekNext() == '=') {
            advance(); advance();
            tokens.push_back(Token{TokenType::GreaterEquals, ">=", 0.0, startLine, startCol});
            continue;
        }

        // Single-character tokens
        advance();
        switch (ch) {
            case '+': tokens.push_back(Token{TokenType::Plus, "+", 0.0, startLine, startCol}); break;
            case '-': tokens.push_back(Token{TokenType::Minus, "-", 0.0, startLine, startCol}); break;
            case '*': tokens.push_back(Token{TokenType::Star, "*", 0.0, startLine, startCol}); break;
            case '/': tokens.push_back(Token{TokenType::Slash, "/", 0.0, startLine, startCol}); break;
            case '^': tokens.push_back(Token{TokenType::Caret, "^", 0.0, startLine, startCol}); break;
            case '=': tokens.push_back(Token{TokenType::Assign, "=", 0.0, startLine, startCol}); break;
            case '<': tokens.push_back(Token{TokenType::Less, "<", 0.0, startLine, startCol}); break;
            case '>': tokens.push_back(Token{TokenType::Greater, ">", 0.0, startLine, startCol}); break;
            case '(': tokens.push_back(Token{TokenType::LParen, "(", 0.0, startLine, startCol}); break;
            case ')': tokens.push_back(Token{TokenType::RParen, ")", 0.0, startLine, startCol}); break;
            case '[': tokens.push_back(Token{TokenType::LBracket, "[", 0.0, startLine, startCol}); break;
            case ']': tokens.push_back(Token{TokenType::RBracket, "]", 0.0, startLine, startCol}); break;
            case ',': tokens.push_back(Token{TokenType::Comma, ",", 0.0, startLine, startCol}); break;
            case ';': tokens.push_back(Token{TokenType::Semicolon, ";", 0.0, startLine, startCol}); break;
            case ':': tokens.push_back(Token{TokenType::Colon, ":", 0.0, startLine, startCol}); break;
            case '\n': tokens.push_back(Token{TokenType::Newline, "\n", 0.0, startLine, startCol}); break;
            default:
                tokens.push_back(Token{TokenType::Invalid, std::string(1, ch), 0.0, startLine, startCol});
                break;
        }
    }

    tokens.push_back(Token{TokenType::EndOfFile, "", 0.0, line_, column_});
    return tokens;
}

} // namespace matforge::engine


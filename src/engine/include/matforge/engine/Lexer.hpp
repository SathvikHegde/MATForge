#pragma once

#include "matforge/engine/Token.hpp"
#include <vector>
#include <string>

namespace matforge::engine {

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;

    void skipWhitespaceAndComments();
    Token readNumber();
    Token readIdentifier();
    Token readString(char quote);

    std::string source_;
    std::size_t cursor_{0};
    std::size_t line_{1};
    std::size_t column_{1};
};

} // namespace matforge::engine


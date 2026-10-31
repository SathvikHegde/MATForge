#pragma once

#include "matforge/engine/Token.hpp"
#include "matforge/engine/Ast.hpp"
#include <vector>
#include <memory>
#include <string>

namespace matforge::engine {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    Program parse();

private:
    const Token& peek() const;
    const Token& peekNext() const;
    const Token& previous() const;
    bool isAtEnd() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchAny(std::initializer_list<TokenType> types);

    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseRelational();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parsePower();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseMatrixLiteral();

    void synchronize();

    std::vector<Token> tokens_;
    std::size_t cursor_{0};
};

} // namespace matforge::engine


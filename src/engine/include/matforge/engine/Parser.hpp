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
    std::unique_ptr<Statement> parseStatementInternal();
    [[noreturn]] void error(const std::string& message) const;
    std::unique_ptr<Statement> parseForStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseFunctionStatement();
    std::vector<std::shared_ptr<Statement>> parseBlockUntil(std::initializer_list<TokenType> endTokens);

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseRelational();
    std::unique_ptr<Expr> parseColon();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parsePower();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseMatrixLiteral();

    bool isSubscriptAssignment() const;
    bool isMultiAssignment() const;
    void consumeSeparators();

    void synchronize();

    std::vector<Token> tokens_;
    std::size_t cursor_{0};
};

} // namespace matforge::engine


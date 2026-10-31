#include "matforge/engine/Parser.hpp"
#include <stdexcept>

namespace matforge::engine {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::peek() const {
    if (cursor_ >= tokens_.size()) {
        static Token eof{TokenType::EndOfFile, "", 0.0, 0, 0};
        return eof;
    }
    return tokens_[cursor_];
}

const Token& Parser::peekNext() const {
    if (cursor_ + 1 >= tokens_.size()) {
        static Token eof{TokenType::EndOfFile, "", 0.0, 0, 0};
        return eof;
    }
    return tokens_[cursor_ + 1];
}

const Token& Parser::previous() const {
    if (cursor_ == 0) return tokens_[0];
    return tokens_[cursor_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::advance() {
    if (!isAtEnd()) cursor_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::matchAny(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Program Parser::parse() {
    Program program;
    while (!isAtEnd()) {
        if (match(TokenType::Newline) || match(TokenType::Semicolon) || match(TokenType::Comma)) {
            continue;
        }
        auto stmt = parseStatement();
        if (stmt) {
            program.statements.push_back(std::move(stmt));
        }
    }
    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    // Check for MATLAB command-line commands without parens (e.g., clear, clc, who, whos, help)
    if (check(TokenType::Identifier)) {
        std::string name = peek().text;
        if (name == "clear" || name == "clc" || name == "who" || name == "whos" || name == "help" ||
            name == "exit" || name == "quit") {
            advance();
            std::vector<std::string> args;
            while (!isAtEnd() && !check(TokenType::Newline) && !check(TokenType::Semicolon) && !check(TokenType::Comma)) {
                if (check(TokenType::Identifier) || check(TokenType::String)) {
                    args.push_back(advance().text);
                } else {
                    break;
                }
            }
            bool suppress = match(TokenType::Semicolon);
            auto cmd = std::make_unique<CommandStmt>(name, args);
            cmd->suppressOutput = suppress;
            return cmd;
        }

        // Check for Assignment: identifier = expr
        if (peekNext().type == TokenType::Assign) {
            std::string varName = advance().text; // consume identifier
            advance(); // consume '='
            auto expr = parseExpression();
            bool suppress = match(TokenType::Semicolon);
            return std::make_unique<AssignmentStmt>(varName, std::move(expr), suppress);
        }
    }

    // Otherwise, Expression statement
    auto expr = parseExpression();
    bool suppress = match(TokenType::Semicolon);
    return std::make_unique<ExpressionStmt>(std::move(expr), suppress);
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseRelational();
}

std::unique_ptr<Expr> Parser::parseRelational() {
    auto expr = parseAdditive();

    while (check(TokenType::Equals) || check(TokenType::NotEquals) ||
           check(TokenType::Less) || check(TokenType::LessEquals) ||
           check(TokenType::Greater) || check(TokenType::GreaterEquals)) {
        TokenType op = advance().type;
        auto right = parseAdditive();
        BinaryOp bOp = BinaryOp::Equal;
        switch (op) {
            case TokenType::Equals: bOp = BinaryOp::Equal; break;
            case TokenType::NotEquals: bOp = BinaryOp::NotEqual; break;
            case TokenType::Less: bOp = BinaryOp::Less; break;
            case TokenType::LessEquals: bOp = BinaryOp::LessEqual; break;
            case TokenType::Greater: bOp = BinaryOp::Greater; break;
            case TokenType::GreaterEquals: bOp = BinaryOp::GreaterEqual; break;
            default: break;
        }
        expr = std::make_unique<BinaryExpr>(bOp, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto expr = parseMultiplicative();

    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        TokenType op = advance().type;
        auto right = parseMultiplicative();
        BinaryOp bOp = (op == TokenType::Plus) ? BinaryOp::Add : BinaryOp::Subtract;
        expr = std::make_unique<BinaryExpr>(bOp, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto expr = parsePower();

    while (check(TokenType::Star) || check(TokenType::Slash) ||
           check(TokenType::DotStar) || check(TokenType::DotSlash)) {
        TokenType op = advance().type;
        auto right = parsePower();
        BinaryOp bOp = BinaryOp::Multiply;
        switch (op) {
            case TokenType::Star: bOp = BinaryOp::Multiply; break;
            case TokenType::Slash: bOp = BinaryOp::Divide; break;
            case TokenType::DotStar: bOp = BinaryOp::DotMultiply; break;
            case TokenType::DotSlash: bOp = BinaryOp::DotDivide; break;
            default: break;
        }
        expr = std::make_unique<BinaryExpr>(bOp, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parsePower() {
    auto expr = parseUnary();

    while (check(TokenType::Caret) || check(TokenType::DotCaret)) {
        TokenType op = advance().type;
        auto right = parseUnary();
        BinaryOp bOp = (op == TokenType::Caret) ? BinaryOp::Power : BinaryOp::DotPower;
        expr = std::make_unique<BinaryExpr>(bOp, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::Minus)) {
        auto operand = parseUnary();
        return std::make_unique<UnaryExpr>(UnaryOp::Negate, std::move(operand));
    }
    if (match(TokenType::Plus)) {
        return parseUnary();
    }
    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();

    while (true) {
        if (match(TokenType::Transpose)) {
            expr = std::make_unique<UnaryExpr>(UnaryOp::Transpose, std::move(expr));
        } else if (match(TokenType::LParen)) {
            // Function call or indexing
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RParen)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::Comma));
            }
            if (!match(TokenType::RParen)) {
                throw std::runtime_error("Expected ')' after function arguments.");
            }

            // If left was a variable, turn into CallExpr
            if (auto varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
                expr = std::make_unique<CallExpr>(varExpr->name(), std::move(args));
            } else {
                throw std::runtime_error("Invalid call expression.");
            }
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::Number)) {
        return std::make_unique<NumberExpr>(previous().numberValue);
    }

    if (match(TokenType::String)) {
        return std::make_unique<StringExpr>(previous().text);
    }

    if (match(TokenType::Identifier)) {
        return std::make_unique<VariableExpr>(previous().text);
    }

    if (match(TokenType::LParen)) {
        auto expr = parseExpression();
        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Expected ')' after expression.");
        }
        return expr;
    }

    if (match(TokenType::LBracket)) {
        return parseMatrixLiteral();
    }

    throw std::runtime_error("Unexpected token in expression: " + peek().text);
}

std::unique_ptr<Expr> Parser::parseMatrixLiteral() {
    std::vector<MatrixLiteralExpr::Row> rows;
    MatrixLiteralExpr::Row currentRow;

    while (!check(TokenType::RBracket) && !isAtEnd()) {
        if (match(TokenType::Semicolon) || match(TokenType::Newline)) {
            if (!currentRow.empty()) {
                rows.push_back(std::move(currentRow));
                currentRow.clear();
            }
            continue;
        }

        currentRow.push_back(parseExpression());

        // Comma is optional between row elements in MATLAB (e.g. [1 2 3] or [1, 2, 3])
        match(TokenType::Comma);
    }

    if (!currentRow.empty()) {
        rows.push_back(std::move(currentRow));
    }

    if (!match(TokenType::RBracket)) {
        throw std::runtime_error("Expected ']' at end of matrix literal.");
    }

    return std::make_unique<MatrixLiteralExpr>(std::move(rows));
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon || previous().type == TokenType::Newline) {
            return;
        }
        advance();
    }
}

} // namespace matforge::engine

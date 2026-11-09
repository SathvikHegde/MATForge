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

void Parser::consumeSeparators() {
    while (check(TokenType::Newline) || check(TokenType::Semicolon) || check(TokenType::Comma)) {
        advance();
    }
}

bool Parser::isSubscriptAssignment() const {
    if (cursor_ + 2 >= tokens_.size()) return false;
    if (tokens_[cursor_].type != TokenType::Identifier) return false;
    if (tokens_[cursor_ + 1].type != TokenType::LParen) return false;

    int depth = 0;
    for (std::size_t i = cursor_ + 1; i < tokens_.size(); ++i) {
        if (tokens_[i].type == TokenType::LParen) {
            depth++;
        } else if (tokens_[i].type == TokenType::RParen) {
            depth--;
            if (depth == 0) {
                return (i + 1 < tokens_.size() && tokens_[i + 1].type == TokenType::Assign);
            }
        }
    }
    return false;
}

bool Parser::isMultiAssignment() const {
    if (cursor_ >= tokens_.size() || tokens_[cursor_].type != TokenType::LBracket) {
        return false;
    }
    std::size_t i = cursor_ + 1;
    bool hasIdent = false;
    while (i < tokens_.size()) {
        if (tokens_[i].type == TokenType::Identifier) {
            hasIdent = true;
            ++i;
        } else if (tokens_[i].type == TokenType::Comma) {
            ++i;
        } else if (tokens_[i].type == TokenType::RBracket) {
            ++i;
            return (i < tokens_.size() && tokens_[i].type == TokenType::Assign && hasIdent);
        } else {
            return false;
        }
    }
    return false;
}

Program Parser::parse() {
    Program program;
    while (!isAtEnd()) {
        consumeSeparators();
        if (isAtEnd()) break;

        auto stmt = parseStatement();
        if (stmt) {
            program.statements.push_back(std::move(stmt));
        }
    }
    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    consumeSeparators();
    if (isAtEnd()) return nullptr;

    // 1. Control flow: For loop
    if (check(TokenType::For)) {
        return parseForStatement();
    }

    // 2. Control flow: While loop
    if (check(TokenType::While)) {
        return parseWhileStatement();
    }

    // 3. Control flow: If statement
    if (check(TokenType::If)) {
        return parseIfStatement();
    }

    // 4. Function definition
    if (check(TokenType::Function)) {
        return parseFunctionStatement();
    }

    // 5. Jump statements
    if (match(TokenType::Break)) {
        match(TokenType::Semicolon);
        return std::make_unique<BreakStmt>();
    }
    if (match(TokenType::Continue)) {
        match(TokenType::Semicolon);
        return std::make_unique<ContinueStmt>();
    }
    if (match(TokenType::Return)) {
        std::unique_ptr<Expr> retExpr = nullptr;
        if (!check(TokenType::Semicolon) && !check(TokenType::Newline) && !isAtEnd()) {
            retExpr = parseExpression();
        }
        match(TokenType::Semicolon);
        return std::make_unique<ReturnStmt>(std::move(retExpr));
    }

    // 6. Multi-Variable Assignment: [U, S, V] = svd(A); or [V, D] = eig(A);
    if (isMultiAssignment()) {
        advance(); // consume '['
        std::vector<std::string> targets;
        while (!check(TokenType::RBracket) && !isAtEnd()) {
            if (check(TokenType::Identifier)) {
                targets.push_back(advance().text);
            } else if (check(TokenType::Comma)) {
                advance();
            } else {
                break;
            }
        }
        if (!match(TokenType::RBracket)) {
            throw std::runtime_error("Expected ']' in multi-assignment.");
        }
        if (!match(TokenType::Assign)) {
            throw std::runtime_error("Expected '=' in multi-assignment.");
        }
        auto expr = parseExpression();
        bool suppress = match(TokenType::Semicolon);
        return std::make_unique<MultiAssignmentStmt>(std::move(targets), std::move(expr), suppress);
    }

    // 7. Subscript Assignment: A(1, 2) = val; or A(:, 1) = val;
    if (isSubscriptAssignment()) {
        std::string varName = advance().text; // consume identifier
        advance(); // consume '('
        std::vector<std::unique_ptr<Expr>> indices;
        if (!check(TokenType::RParen)) {
            do {
                if (check(TokenType::Colon) && 
                    (peekNext().type == TokenType::Comma || peekNext().type == TokenType::RParen)) {
                    advance();
                    indices.push_back(std::make_unique<ColonAllExpr>());
                } else {
                    indices.push_back(parseExpression());
                }
            } while (match(TokenType::Comma));
        }
        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Expected ')' after subscript indices.");
        }
        if (!match(TokenType::Assign)) {
            throw std::runtime_error("Expected '=' in subscript assignment.");
        }
        auto expr = parseExpression();
        bool suppress = match(TokenType::Semicolon);
        return std::make_unique<SubscriptAssignStmt>(varName, std::move(indices), std::move(expr), suppress);
    }

    // 7. Check for MATLAB command-line commands without parens (e.g., clear, clc, who, whos, help, close, clf, run)
    if (check(TokenType::Identifier)) {
        std::string name = peek().text;
        if (name == "clear" || name == "clc" || name == "who" || name == "whos" || name == "help" ||
            name == "exit" || name == "quit" || name == "close" || name == "clf" || name == "run" ||
            name == "hold" || name == "grid") {
            advance();
            std::vector<std::string> args;
            while (!isAtEnd() && !check(TokenType::Newline) && !check(TokenType::Semicolon) && !check(TokenType::Comma)) {
                if (check(TokenType::Identifier) || check(TokenType::String) || check(TokenType::Number)) {
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

        // 8. Variable Assignment: identifier = expr
        if (peekNext().type == TokenType::Assign) {
            std::string varName = advance().text; // consume identifier
            advance(); // consume '='
            auto expr = parseExpression();
            bool suppress = match(TokenType::Semicolon);
            return std::make_unique<AssignmentStmt>(varName, std::move(expr), suppress);
        }
    }

    // 9. Otherwise, Expression statement
    auto expr = parseExpression();
    bool suppress = match(TokenType::Semicolon);
    return std::make_unique<ExpressionStmt>(std::move(expr), suppress);
}

std::unique_ptr<Statement> Parser::parseForStatement() {
    advance(); // consume 'for'
    if (!check(TokenType::Identifier)) {
        throw std::runtime_error("Expected variable name after 'for'.");
    }
    std::string varName = advance().text;
    if (!match(TokenType::Assign)) {
        throw std::runtime_error("Expected '=' after for loop variable.");
    }
    auto range = parseExpression();
    consumeSeparators();
    auto body = parseBlockUntil({TokenType::End});
    if (!match(TokenType::End)) {
        throw std::runtime_error("Expected 'end' at end of for loop.");
    }
    match(TokenType::Semicolon);
    return std::make_unique<ForStmt>(varName, std::move(range), std::move(body));
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    advance(); // consume 'while'
    auto cond = parseExpression();
    consumeSeparators();
    auto body = parseBlockUntil({TokenType::End});
    if (!match(TokenType::End)) {
        throw std::runtime_error("Expected 'end' at end of while loop.");
    }
    match(TokenType::Semicolon);
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
    advance(); // consume 'if'
    std::vector<IfBranch> branches;
    auto cond = parseExpression();
    consumeSeparators();
    auto body = parseBlockUntil({TokenType::Elseif, TokenType::Else, TokenType::End});
    branches.push_back(IfBranch{std::move(cond), std::move(body)});

    while (match(TokenType::Elseif)) {
        auto elifCond = parseExpression();
        consumeSeparators();
        auto elifBody = parseBlockUntil({TokenType::Elseif, TokenType::Else, TokenType::End});
        branches.push_back(IfBranch{std::move(elifCond), std::move(elifBody)});
    }

    std::vector<std::shared_ptr<Statement>> elseBody;
    if (match(TokenType::Else)) {
        consumeSeparators();
        elseBody = parseBlockUntil({TokenType::End});
    }

    if (!match(TokenType::End)) {
        throw std::runtime_error("Expected 'end' at end of if statement.");
    }
    match(TokenType::Semicolon);
    return std::make_unique<IfStmt>(std::move(branches), std::move(elseBody));
}

std::unique_ptr<Statement> Parser::parseFunctionStatement() {
    advance(); // consume 'function'
    std::vector<std::string> returnParams;
    std::string functionName;

    // Check if bracketed return list: [a, b] = f(...)
    if (match(TokenType::LBracket)) {
        while (!check(TokenType::RBracket) && !isAtEnd()) {
            if (check(TokenType::Identifier)) {
                returnParams.push_back(advance().text);
            }
            match(TokenType::Comma);
        }
        if (!match(TokenType::RBracket) || !match(TokenType::Assign)) {
            throw std::runtime_error("Expected '] =' after function return parameter list.");
        }
        if (!check(TokenType::Identifier)) {
            throw std::runtime_error("Expected function name after return parameters.");
        }
        functionName = advance().text;
    } else if (check(TokenType::Identifier)) {
        std::string firstId = advance().text;
        if (match(TokenType::Assign)) {
            // function a = f(...)
            returnParams.push_back(firstId);
            if (!check(TokenType::Identifier)) {
                throw std::runtime_error("Expected function name after '=' in function declaration.");
            }
            functionName = advance().text;
        } else {
            // function f(...)
            functionName = firstId;
        }
    } else {
        throw std::runtime_error("Expected function name or return list in function declaration.");
    }

    // Parameter list: (p1, p2, ...)
    std::vector<std::string> paramNames;
    if (match(TokenType::LParen)) {
        if (!check(TokenType::RParen)) {
            do {
                if (check(TokenType::Identifier)) {
                    paramNames.push_back(advance().text);
                }
            } while (match(TokenType::Comma));
        }
        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Expected ')' after function parameter list.");
        }
    }

    consumeSeparators();
    auto body = parseBlockUntil({TokenType::End});
    if (match(TokenType::End)) {
        match(TokenType::Semicolon);
    }

    return std::make_unique<FunctionDefStmt>(functionName, returnParams, paramNames, std::move(body));
}

std::vector<std::shared_ptr<Statement>> Parser::parseBlockUntil(std::initializer_list<TokenType> endTokens) {
    std::vector<std::shared_ptr<Statement>> stmts;
    while (!isAtEnd()) {
        consumeSeparators();
        if (isAtEnd()) break;

        bool shouldStop = false;
        for (TokenType et : endTokens) {
            if (check(et)) {
                shouldStop = true;
                break;
            }
        }
        if (shouldStop) break;

        auto stmt = parseStatement();
        if (stmt) {
            stmts.push_back(std::move(stmt));
        }
    }
    return stmts;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseRelational();
}

std::unique_ptr<Expr> Parser::parseRelational() {
    auto expr = parseColon();

    while (check(TokenType::Equals) || check(TokenType::NotEquals) ||
           check(TokenType::Less) || check(TokenType::LessEquals) ||
           check(TokenType::Greater) || check(TokenType::GreaterEquals)) {
        TokenType op = advance().type;
        auto right = parseColon();
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

std::unique_ptr<Expr> Parser::parseColon() {
    auto expr = parseAdditive();

    if (match(TokenType::Colon)) {
        auto next = parseAdditive();
        if (match(TokenType::Colon)) {
            // start:step:stop
            auto stop = parseAdditive();
            return std::make_unique<RangeExpr>(std::move(expr), std::move(next), std::move(stop));
        } else {
            // start:stop
            return std::make_unique<RangeExpr>(std::move(expr), nullptr, std::move(next));
        }
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

    while (check(TokenType::Star) || check(TokenType::Slash) || check(TokenType::Backslash) ||
           check(TokenType::DotStar) || check(TokenType::DotSlash) || check(TokenType::DotBackslash)) {
        TokenType op = advance().type;
        auto right = parsePower();
        BinaryOp bOp = BinaryOp::Multiply;
        switch (op) {
            case TokenType::Star: bOp = BinaryOp::Multiply; break;
            case TokenType::Slash: bOp = BinaryOp::Divide; break;
            case TokenType::Backslash: bOp = BinaryOp::Backslash; break;
            case TokenType::DotStar: bOp = BinaryOp::DotMultiply; break;
            case TokenType::DotSlash: bOp = BinaryOp::DotDivide; break;
            case TokenType::DotBackslash: bOp = BinaryOp::DotBackslash; break;
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
                    if (check(TokenType::Colon) && 
                        (peekNext().type == TokenType::Comma || peekNext().type == TokenType::RParen)) {
                        advance();
                        args.push_back(std::make_unique<ColonAllExpr>());
                    } else {
                        args.push_back(parseExpression());
                    }
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

    if (match(TokenType::End)) {
        return std::make_unique<EndExpr>();
    }

    if (match(TokenType::Colon)) {
        return std::make_unique<ColonAllExpr>();
    }

    if (match(TokenType::At)) {
        // Function handle: @(x) expr or @sin
        if (match(TokenType::LParen)) {
            std::vector<std::string> paramNames;
            if (!check(TokenType::RParen)) {
                do {
                    if (!check(TokenType::Identifier)) {
                        throw std::runtime_error("Expected parameter name in anonymous function.");
                    }
                    paramNames.push_back(advance().text);
                } while (match(TokenType::Comma));
            }
            if (!match(TokenType::RParen)) {
                throw std::runtime_error("Expected ')' after anonymous function parameters.");
            }
            auto body = parseExpression();
            return std::make_unique<FunctionHandleExpr>(std::move(paramNames), std::move(body));
        } else if (match(TokenType::Identifier)) {
            return std::make_unique<FunctionHandleExpr>(previous().text);
        } else {
            throw std::runtime_error("Expected '(' or function name after '@'.");
        }
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

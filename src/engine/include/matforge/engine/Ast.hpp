#pragma once

#include "matforge/core/Value.hpp"
#include <string>
#include <vector>
#include <memory>

namespace matforge::engine {

// Base Expression
class Expr {
public:
    virtual ~Expr() = default;
};

// Literals
class NumberExpr : public Expr {
public:
    explicit NumberExpr(double value) : value_(value) {}
    double value() const { return value_; }
private:
    double value_;
};

class StringExpr : public Expr {
public:
    explicit StringExpr(std::string value) : value_(std::move(value)) {}
    const std::string& value() const { return value_; }
private:
    std::string value_;
};

class VariableExpr : public Expr {
public:
    explicit VariableExpr(std::string name) : name_(std::move(name)) {}
    const std::string& name() const { return name_; }
private:
    std::string name_;
};

// Matrix Literal: [1, 2; 3, 4]
class MatrixLiteralExpr : public Expr {
public:
    using Row = std::vector<std::unique_ptr<Expr>>;
    explicit MatrixLiteralExpr(std::vector<Row> rows) : rows_(std::move(rows)) {}
    const std::vector<Row>& rows() const { return rows_; }
private:
    std::vector<Row> rows_;
};

// Binary Expressions
enum class BinaryOp {
    Add,
    Subtract,
    Multiply,
    Divide,
    Power,
    DotMultiply,
    DotDivide,
    DotPower,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : op_(op), left_(std::move(left)), right_(std::move(right)) {}

    BinaryOp op() const { return op_; }
    const Expr& left() const { return *left_; }
    const Expr& right() const { return *right_; }

private:
    BinaryOp op_;
    std::unique_ptr<Expr> left_;
    std::unique_ptr<Expr> right_;
};

// Unary Expressions (+, -, ')
enum class UnaryOp {
    Plus,
    Negate,
    Transpose
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand)
        : op_(op), operand_(std::move(operand)) {}

    UnaryOp op() const { return op_; }
    const Expr& operand() const { return *operand_; }

private:
    UnaryOp op_;
    std::unique_ptr<Expr> operand_;
};

// Function Call or Array Indexing: f(x, y)
class CallExpr : public Expr {
public:
    CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args)
        : callee_(std::move(callee)), args_(std::move(args)) {}

    const std::string& callee() const { return callee_; }
    const std::vector<std::unique_ptr<Expr>>& args() const { return args_; }

private:
    std::string callee_;
    std::vector<std::unique_ptr<Expr>> args_;
};

// Base Statement
class Statement {
public:
    virtual ~Statement() = default;
    bool suppressOutput{false};
};

class AssignmentStmt : public Statement {
public:
    AssignmentStmt(std::string variableName, std::unique_ptr<Expr> expr, bool suppress)
        : variableName_(std::move(variableName)), expr_(std::move(expr)) {
        suppressOutput = suppress;
    }

    const std::string& variableName() const { return variableName_; }
    const Expr& expr() const { return *expr_; }

private:
    std::string variableName_;
    std::unique_ptr<Expr> expr_;
};

class ExpressionStmt : public Statement {
public:
    explicit ExpressionStmt(std::unique_ptr<Expr> expr, bool suppress)
        : expr_(std::move(expr)) {
        suppressOutput = suppress;
    }

    const Expr& expr() const { return *expr_; }

private:
    std::unique_ptr<Expr> expr_;
};

class CommandStmt : public Statement {
public:
    explicit CommandStmt(std::string command, std::vector<std::string> args = {})
        : command_(std::move(command)), args_(std::move(args)) {
        suppressOutput = false;
    }

    const std::string& command() const { return command_; }
    const std::vector<std::string>& args() const { return args_; }

private:
    std::string command_;
    std::vector<std::string> args_;
};

struct Program {
    std::vector<std::unique_ptr<Statement>> statements;
};

} // namespace matforge::engine


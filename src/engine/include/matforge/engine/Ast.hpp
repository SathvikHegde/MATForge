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

// Function Handle / Anonymous Function: @(x) x.^2 or @sin
class FunctionHandleExpr : public Expr {
public:
    FunctionHandleExpr(std::vector<std::string> paramNames, 
                       std::shared_ptr<Expr> body,
                       std::string representation = "")
        : paramNames_(std::move(paramNames)), 
          body_(std::move(body)),
          representation_(std::move(representation)) {}

    explicit FunctionHandleExpr(std::string functionName)
        : namedFunction_(std::move(functionName)), isNamed_(true) {}

    bool isNamed() const { return isNamed_; }
    const std::string& namedFunction() const { return namedFunction_; }
    const std::vector<std::string>& paramNames() const { return paramNames_; }
    const Expr* body() const { return body_.get(); }
    std::shared_ptr<Expr> sharedBody() const { return body_; }
    const std::string& representation() const { return representation_; }

private:
    std::vector<std::string> paramNames_;
    std::shared_ptr<Expr> body_;
    std::string namedFunction_;
    std::string representation_;
    bool isNamed_{false};
};

// Binary Expressions
enum class BinaryOp {
    Add,
    Subtract,
    Multiply,
    Divide,
    Backslash,
    Power,
    DotMultiply,
    DotDivide,
    DotBackslash,
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

// Range Expression: start:stop or start:step:stop
class RangeExpr : public Expr {
public:
    RangeExpr(std::unique_ptr<Expr> start, std::unique_ptr<Expr> step, std::unique_ptr<Expr> stop)
        : start_(std::move(start)), step_(std::move(step)), stop_(std::move(stop)) {}

    const Expr& start() const { return *start_; }
    bool hasStep() const { return step_ != nullptr; }
    const Expr* step() const { return step_.get(); }
    const Expr& stop() const { return *stop_; }

private:
    std::unique_ptr<Expr> start_;
    std::unique_ptr<Expr> step_;
    std::unique_ptr<Expr> stop_;
};

// Standalone colon index (e.g., A(:, 1))
class ColonAllExpr : public Expr {
public:
    ColonAllExpr() = default;
};

// End index keyword (e.g., A(2:end))
class EndExpr : public Expr {
public:
    EndExpr() = default;
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

// Subscripted assignment: A(1, 2) = val; or A(:, 1) = val;
class SubscriptAssignStmt : public Statement {
public:
    SubscriptAssignStmt(std::string variableName, 
                        std::vector<std::unique_ptr<Expr>> indices, 
                        std::unique_ptr<Expr> expr, 
                        bool suppress)
        : variableName_(std::move(variableName)), 
          indices_(std::move(indices)), 
          expr_(std::move(expr)) {
        suppressOutput = suppress;
    }

    const std::string& variableName() const { return variableName_; }
    const std::vector<std::unique_ptr<Expr>>& indices() const { return indices_; }
    const Expr& expr() const { return *expr_; }

private:
    std::string variableName_;
    std::vector<std::unique_ptr<Expr>> indices_;
    std::unique_ptr<Expr> expr_;
};

// For loop: for var = range ... end
class ForStmt : public Statement {
public:
    ForStmt(std::string variableName, 
            std::unique_ptr<Expr> rangeExpr, 
            std::vector<std::shared_ptr<Statement>> body)
        : variableName_(std::move(variableName)), 
          rangeExpr_(std::move(rangeExpr)), 
          body_(std::move(body)) {}

    const std::string& variableName() const { return variableName_; }
    const Expr& rangeExpr() const { return *rangeExpr_; }
    const std::vector<std::shared_ptr<Statement>>& body() const { return body_; }

private:
    std::string variableName_;
    std::unique_ptr<Expr> rangeExpr_;
    std::vector<std::shared_ptr<Statement>> body_;
};

// While loop: while condition ... end
class WhileStmt : public Statement {
public:
    WhileStmt(std::unique_ptr<Expr> condition, 
              std::vector<std::shared_ptr<Statement>> body)
        : condition_(std::move(condition)), 
          body_(std::move(body)) {}

    const Expr& condition() const { return *condition_; }
    const std::vector<std::shared_ptr<Statement>>& body() const { return body_; }

private:
    std::unique_ptr<Expr> condition_;
    std::vector<std::shared_ptr<Statement>> body_;
};

// If statement: if cond ... elseif cond ... else ... end
struct IfBranch {
    std::unique_ptr<Expr> condition;
    std::vector<std::shared_ptr<Statement>> body;
};

class IfStmt : public Statement {
public:
    IfStmt(std::vector<IfBranch> branches, 
           std::vector<std::shared_ptr<Statement>> elseBody)
        : branches_(std::move(branches)), 
          elseBody_(std::move(elseBody)) {}

    const std::vector<IfBranch>& branches() const { return branches_; }
    const std::vector<std::shared_ptr<Statement>>& elseBody() const { return elseBody_; }

private:
    std::vector<IfBranch> branches_;
    std::vector<std::shared_ptr<Statement>> elseBody_;
};

// Loop control statements
class BreakStmt : public Statement {
public:
    BreakStmt() = default;
};

class ContinueStmt : public Statement {
public:
    ContinueStmt() = default;
};

class ReturnStmt : public Statement {
public:
    explicit ReturnStmt(std::unique_ptr<Expr> expr = nullptr) : expr_(std::move(expr)) {}
    const Expr* expr() const { return expr_.get(); }
private:
    std::unique_ptr<Expr> expr_;
};

// User function definition: function [out1, out2] = name(in1, in2) ... end
class FunctionDefStmt : public Statement {
public:
    FunctionDefStmt(std::string functionName,
                    std::vector<std::string> returnParams,
                    std::vector<std::string> paramNames,
                    std::vector<std::shared_ptr<Statement>> body)
        : functionName_(std::move(functionName)),
          returnParams_(std::move(returnParams)),
          paramNames_(std::move(paramNames)),
          body_(std::move(body)) {}

    const std::string& functionName() const { return functionName_; }
    const std::vector<std::string>& returnParams() const { return returnParams_; }
    const std::vector<std::string>& paramNames() const { return paramNames_; }
    const std::vector<std::shared_ptr<Statement>>& body() const { return body_; }

private:
    std::string functionName_;
    std::vector<std::string> returnParams_;
    std::vector<std::string> paramNames_;
    std::vector<std::shared_ptr<Statement>> body_;
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

class MultiAssignmentStmt : public Statement {
public:
    MultiAssignmentStmt(std::vector<std::string> targetVars, std::unique_ptr<Expr> expr, bool suppress)
        : targetVars_(std::move(targetVars)), expr_(std::move(expr)) {
        suppressOutput = suppress;
    }

    const std::vector<std::string>& targetVars() const { return targetVars_; }
    const Expr& expr() const { return *expr_; }

private:
    std::vector<std::string> targetVars_;
    std::unique_ptr<Expr> expr_;
};

struct Program {
    std::vector<std::unique_ptr<Statement>> statements;
};

} // namespace matforge::engine


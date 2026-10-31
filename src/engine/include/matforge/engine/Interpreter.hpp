#pragma once

#include "matforge/core/Value.hpp"
#include "matforge/engine/Workspace.hpp"
#include "matforge/engine/Ast.hpp"
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace matforge::engine {

struct ExecutionResult {
    bool success{true};
    std::string output;
    std::string errorMessage;
};

class Interpreter {
public:
    using PlotCallback = std::function<void(const std::vector<double>& x, 
                                            const std::vector<double>& y, 
                                            const std::string& title)>;
    using ClearScreenCallback = std::function<void()>;

    explicit Interpreter(std::shared_ptr<Workspace> workspace = nullptr);

    ExecutionResult execute(const std::string& code);

    std::shared_ptr<Workspace> workspace() const { return workspace_; }

    void setPlotCallback(PlotCallback cb) { plotCallback_ = std::move(cb); }
    void setClearScreenCallback(ClearScreenCallback cb) { clearScreenCallback_ = std::move(cb); }

private:
    matforge::core::Value evaluate(const Expr& expr);
    matforge::core::Value evaluateNumber(const NumberExpr& expr);
    matforge::core::Value evaluateString(const StringExpr& expr);
    matforge::core::Value evaluateVariable(const VariableExpr& expr);
    matforge::core::Value evaluateMatrixLiteral(const MatrixLiteralExpr& expr);
    matforge::core::Value evaluateBinary(const BinaryExpr& expr);
    matforge::core::Value evaluateUnary(const UnaryExpr& expr);
    matforge::core::Value evaluateCall(const CallExpr& expr);

    void executeStatement(const Statement& stmt, std::ostringstream& out);

    std::shared_ptr<Workspace> workspace_;
    PlotCallback plotCallback_;
    ClearScreenCallback clearScreenCallback_;
};

} // namespace matforge::engine


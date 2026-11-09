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
    struct PlotCommand {
        std::vector<double> x;
        std::vector<double> y;
        std::string format;
        std::string plotType{"line"};
        std::string label;
    };

    using PlotCallback = std::function<void(const PlotCommand& cmd)>;
    using HoldCallback = std::function<void(bool hold)>;
    using GridCallback = std::function<void(bool grid)>;
    using TitleCallback = std::function<void(const std::string& title)>;
    using LabelCallback = std::function<void(const std::string& label, bool isX)>;
    using LegendCallback = std::function<void(const std::vector<std::string>& labels)>;
    using SubplotCallback = std::function<void(int rows, int cols, int index)>;
    using ClearScreenCallback = std::function<void()>;
    using CloseFigureCallback = std::function<void()>;
    using ClearFigureCallback = std::function<void()>;

    explicit Interpreter(std::shared_ptr<Workspace> workspace = nullptr);

    ExecutionResult execute(const std::string& code);

    std::shared_ptr<Workspace> workspace() const { return workspace_; }

    void setPlotCallback(PlotCallback cb) { plotCallback_ = std::move(cb); }
    void setHoldCallback(HoldCallback cb) { holdCallback_ = std::move(cb); }
    void setGridCallback(GridCallback cb) { gridCallback_ = std::move(cb); }
    void setTitleCallback(TitleCallback cb) { titleCallback_ = std::move(cb); }
    void setLabelCallback(LabelCallback cb) { labelCallback_ = std::move(cb); }
    void setLegendCallback(LegendCallback cb) { legendCallback_ = std::move(cb); }
    void setSubplotCallback(SubplotCallback cb) { subplotCallback_ = std::move(cb); }
    void setClearScreenCallback(ClearScreenCallback cb) { clearScreenCallback_ = std::move(cb); }
    void setCloseFigureCallback(CloseFigureCallback cb) { closeFigureCallback_ = std::move(cb); }
    void setClearFigureCallback(ClearFigureCallback cb) { clearFigureCallback_ = std::move(cb); }

    const PlotCallback& plotCallback() const { return plotCallback_; }
    const HoldCallback& holdCallback() const { return holdCallback_; }
    const GridCallback& gridCallback() const { return gridCallback_; }
    const TitleCallback& titleCallback() const { return titleCallback_; }
    const LabelCallback& labelCallback() const { return labelCallback_; }
    const LegendCallback& legendCallback() const { return legendCallback_; }
    const SubplotCallback& subplotCallback() const { return subplotCallback_; }
    const ClearScreenCallback& clearScreenCallback() const { return clearScreenCallback_; }
    const CloseFigureCallback& closeFigureCallback() const { return closeFigureCallback_; }
    const ClearFigureCallback& clearFigureCallback() const { return clearFigureCallback_; }

private:
    matforge::core::Value evaluate(const Expr& expr);
    matforge::core::Value evaluateNumber(const NumberExpr& expr);
    matforge::core::Value evaluateString(const StringExpr& expr);
    matforge::core::Value evaluateVariable(const VariableExpr& expr);
    matforge::core::Value evaluateMatrixLiteral(const MatrixLiteralExpr& expr);
    matforge::core::Value evaluateBinary(const BinaryExpr& expr);
    matforge::core::Value evaluateUnary(const UnaryExpr& expr);
    matforge::core::Value evaluateCall(const CallExpr& expr);
    matforge::core::Value evaluateFunctionHandle(const FunctionHandleExpr& expr);
    matforge::core::Value invokeFunction(const std::string& name, const std::vector<matforge::core::Value>& argValues);
    matforge::core::Value evaluateRange(const RangeExpr& expr);
    matforge::core::Value evaluateEnd(const EndExpr& expr);

    void executeStatement(const Statement& stmt, std::ostringstream& out);
    void executeMultiAssign(const MultiAssignmentStmt& stmt, std::ostringstream& out);
    void executeSubscriptAssign(const SubscriptAssignStmt& stmt, std::ostringstream& out);
    void executeFor(const ForStmt& stmt, std::ostringstream& out);
    void executeWhile(const WhileStmt& stmt, std::ostringstream& out);
    void executeIf(const IfStmt& stmt, std::ostringstream& out);
    void executeFunctionDef(const FunctionDefStmt& stmt, std::ostringstream& out);

    std::vector<std::size_t> resolveIndexExpr(const Expr& expr, std::size_t dimSize);

    std::shared_ptr<Workspace> workspace_;
    std::vector<std::size_t> endContextStack_;
    PlotCallback plotCallback_;
    HoldCallback holdCallback_;
    GridCallback gridCallback_;
    TitleCallback titleCallback_;
    LabelCallback labelCallback_;
    LegendCallback legendCallback_;
    SubplotCallback subplotCallback_;
    ClearScreenCallback clearScreenCallback_;
    CloseFigureCallback closeFigureCallback_;
    ClearFigureCallback clearFigureCallback_;
};

} // namespace matforge::engine


#include "matforge/engine/Interpreter.hpp"
#include "matforge/engine/Lexer.hpp"
#include "matforge/engine/Parser.hpp"
#include "matforge/core/MathUtils.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace matforge::engine {

namespace {
struct BreakException {};
struct ContinueException {};
struct ReturnException {
    matforge::core::Value value;
};
}

Interpreter::Interpreter(std::shared_ptr<Workspace> workspace)
    : workspace_(workspace ? workspace : std::make_shared<Workspace>()) {}

ExecutionResult Interpreter::execute(const std::string& code) {
    ExecutionResult result;
    std::ostringstream out;

    try {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        Program program = parser.parse();

        for (const auto& stmt : program.statements) {
            executeStatement(*stmt, out);
        }

        result.success = true;
        result.output = out.str();
    } catch (const ReturnException&) {
        // Return at top-level script
        result.success = true;
        result.output = out.str();
    } catch (const BreakException&) {
        result.success = false;
        result.errorMessage = "Break statement outside of loop.";
    } catch (const ContinueException&) {
        result.success = false;
        result.errorMessage = "Continue statement outside of loop.";
    } catch (const std::exception& ex) {
        result.success = false;
        result.errorMessage = ex.what();
    }

    return result;
}

void Interpreter::executeStatement(const Statement& stmt, std::ostringstream& out) {
    if (const auto* assignStmt = dynamic_cast<const AssignmentStmt*>(&stmt)) {
        matforge::core::Value val = evaluate(assignStmt->expr());
        workspace_->set(assignStmt->variableName(), val);

        if (!assignStmt->suppressOutput && !val.isEmpty()) {
            out << assignStmt->variableName() << " =\n\n" << val.formatOutput() << "\n\n";
        }
    } else if (const auto* multiAssign = dynamic_cast<const MultiAssignmentStmt*>(&stmt)) {
        executeMultiAssign(*multiAssign, out);
    } else if (const auto* subAssign = dynamic_cast<const SubscriptAssignStmt*>(&stmt)) {
        executeSubscriptAssign(*subAssign, out);
    } else if (const auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        executeFor(*forStmt, out);
    } else if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        executeWhile(*whileStmt, out);
    } else if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        executeIf(*ifStmt, out);
    } else if (const auto* funcDef = dynamic_cast<const FunctionDefStmt*>(&stmt)) {
        executeFunctionDef(*funcDef, out);
    } else if (dynamic_cast<const BreakStmt*>(&stmt)) {
        throw BreakException{};
    } else if (dynamic_cast<const ContinueStmt*>(&stmt)) {
        throw ContinueException{};
    } else if (const auto* retStmt = dynamic_cast<const ReturnStmt*>(&stmt)) {
        matforge::core::Value retVal;
        if (retStmt->expr()) {
            retVal = evaluate(*retStmt->expr());
        }
        throw ReturnException{std::move(retVal)};
    } else if (const auto* exprStmt = dynamic_cast<const ExpressionStmt*>(&stmt)) {
        matforge::core::Value val = evaluate(exprStmt->expr());
        workspace_->set("ans", val);

        if (!exprStmt->suppressOutput && !val.isEmpty()) {
            out << "ans =\n\n" << val.formatOutput() << "\n\n";
        }
    } else if (const auto* cmdStmt = dynamic_cast<const CommandStmt*>(&stmt)) {
        const std::string& cmd = cmdStmt->command();
        if (cmd == "clear") {
            if (cmdStmt->args().empty()) {
                workspace_->clear();
            } else {
                for (const auto& var : cmdStmt->args()) {
                    workspace_->remove(var);
                }
            }
        } else if (cmd == "clc") {
            if (clearScreenCallback_) {
                clearScreenCallback_();
            }
        } else if (cmd == "who") {
            out << "Your variables are:\n\n";
            for (const auto& [name, _] : workspace_->variables()) {
                if (name == "pi" || name == "e" || name == "eps" || name == "Inf" || name == "NaN") continue;
                out << "  " << name;
            }
            out << "\n\n";
        } else if (cmd == "whos") {
            out << "  Name      Size         Bytes  Class\n";
            out << "  ====      ====         =====  =====\n";
            for (const auto& [name, val] : workspace_->variables()) {
                if (name == "pi" || name == "e" || name == "eps" || name == "Inf" || name == "NaN") continue;
                out << "  " << std::left << std::setw(10) << name
                    << std::setw(13) << val.sizeString()
                    << std::setw(7) << val.byteSize()
                    << val.typeName() << "\n";
            }
            out << "\n";
        } else if (cmd == "help") {
            out << "MATForge MATLAB Clone - Interactive REPL\n"
                << "Supported commands: clear, clc, who, whos, help, exit, quit, close, clf, run\n"
                << "Control flow: for, while, if/elseif/else, function, break, continue, return\n"
                << "Built-in functions: zeros, ones, eye, linspace, sin, cos, tan, sqrt, exp, log, abs, det, trace, sum, mean, min, max, size, length, disp, plot\n\n";
        } else if (cmd == "close") {
            if (closeFigureCallback_) {
                closeFigureCallback_();
            }
        } else if (cmd == "clf") {
            if (clearFigureCallback_) {
                clearFigureCallback_();
            }
        } else if (cmd == "hold") {
            bool h = true;
            if (!cmdStmt->args().empty() && cmdStmt->args()[0] == "off") {
                h = false;
            }
            if (holdCallback_) {
                holdCallback_(h);
            }
        } else if (cmd == "grid") {
            bool g = true;
            if (!cmdStmt->args().empty() && cmdStmt->args()[0] == "off") {
                g = false;
            }
            if (gridCallback_) {
                gridCallback_(g);
            }
        } else if (cmd == "run") {
            if (cmdStmt->args().empty()) {
                out << "Usage: run <script.m>\n\n";
            } else {
                std::string path = cmdStmt->args()[0];
                std::ifstream f(path);
                if (!f.is_open() && path.find('.') == std::string::npos) {
                    f.open(path + ".m");
                }
                if (!f.is_open()) {
                    out << "Error: Could not open script file '" << path << "'\n\n";
                } else {
                    std::stringstream ss;
                    ss << f.rdbuf();
                    auto subRes = execute(ss.str());
                    if (!subRes.success) {
                        out << "Error in " << path << ": " << subRes.errorMessage << "\n\n";
                    } else {
                        out << subRes.output;
                    }
                }
            }
        } else if (cmd == "exit" || cmd == "quit") {
            std::exit(0);
        }
    }
}

void Interpreter::executeMultiAssign(const MultiAssignmentStmt& stmt, std::ostringstream& out) {
    const auto& targets = stmt.targetVars();
    if (targets.empty()) return;

    std::vector<matforge::core::Value> returnValues;

    // Check if expr is a CallExpr to a multi-output decomposition or function
    if (const auto* call = dynamic_cast<const CallExpr*>(&stmt.expr())) {
        const std::string& name = call->callee();
        std::vector<matforge::core::Value> args;
        for (const auto& a : call->args()) {
            args.push_back(evaluate(*a));
        }

        if (name == "svd" && !args.empty() && args[0].isMatrix()) {
            auto [U, S, V] = args[0].toMatrix().svd();
            returnValues.push_back(matforge::core::Value(U));
            returnValues.push_back(matforge::core::Value(S));
            returnValues.push_back(matforge::core::Value(V));
        } else if (name == "eig" && !args.empty() && args[0].isMatrix()) {
            auto [V, D] = args[0].toMatrix().eig();
            returnValues.push_back(matforge::core::Value(V));
            returnValues.push_back(matforge::core::Value(D));
        } else if (name == "lu" && !args.empty() && args[0].isMatrix()) {
            auto [L, U] = args[0].toMatrix().lu();
            returnValues.push_back(matforge::core::Value(L));
            returnValues.push_back(matforge::core::Value(U));
        } else if (name == "qr" && !args.empty() && args[0].isMatrix()) {
            auto [Q, R] = args[0].toMatrix().qr();
            returnValues.push_back(matforge::core::Value(Q));
            returnValues.push_back(matforge::core::Value(R));
        } else if (workspace_->hasFunction(name)) {
            // User-defined function with multiple return params
            auto func = workspace_->getFunction(name);
            auto localWs = std::make_shared<Workspace>();
            for (const auto& [fName, fDef] : workspace_->functions()) {
                localWs->defineFunction(fName, fDef);
            }
            for (std::size_t i = 0; i < func->paramNames().size(); ++i) {
                if (i < args.size()) {
                    localWs->set(func->paramNames()[i], args[i]);
                } else {
                    localWs->set(func->paramNames()[i], matforge::core::Value());
                }
            }

            Interpreter localInterp(localWs);
            localInterp.setPlotCallback(plotCallback_);
            localInterp.setHoldCallback(holdCallback_);
            localInterp.setGridCallback(gridCallback_);
            localInterp.setTitleCallback(titleCallback_);
            localInterp.setLabelCallback(labelCallback_);
            localInterp.setLegendCallback(legendCallback_);
            localInterp.setSubplotCallback(subplotCallback_);
            localInterp.setClearScreenCallback(clearScreenCallback_);
            localInterp.setCloseFigureCallback(closeFigureCallback_);
            localInterp.setClearFigureCallback(clearFigureCallback_);

            std::ostringstream localOut;
            try {
                for (const auto& s : func->body()) {
                    localInterp.executeStatement(*s, localOut);
                }
            } catch (const ReturnException&) {
                // Return executed
            }

            for (const auto& rParam : func->returnParams()) {
                if (localWs->has(rParam)) {
                    returnValues.push_back(localWs->get(rParam));
                } else {
                    returnValues.push_back(matforge::core::Value());
                }
            }
        }
    }

    if (returnValues.empty()) {
        matforge::core::Value singleVal = evaluate(stmt.expr());
        returnValues.push_back(singleVal);
    }

    for (std::size_t i = 0; i < targets.size(); ++i) {
        matforge::core::Value val = (i < returnValues.size()) ? returnValues[i] : matforge::core::Value();
        workspace_->set(targets[i], val);
        if (!stmt.suppressOutput && !val.isEmpty()) {
            out << targets[i] << " =\n\n" << val.formatOutput() << "\n\n";
        }
    }
}

void Interpreter::executeSubscriptAssign(const SubscriptAssignStmt& stmt, std::ostringstream& out) {
    const std::string& name = stmt.variableName();
    matforge::core::Matrix mat;
    if (workspace_->has(name)) {
        const auto& existing = workspace_->get(name);
        if (existing.isNumeric()) {
            mat = existing.toMatrix();
        }
    }

    matforge::core::Value rhsVal = evaluate(stmt.expr());
    matforge::core::Matrix rhsMat = rhsVal.toMatrix();

    if (stmt.indices().size() == 1) {
        auto indices = resolveIndexExpr(*stmt.indices()[0], mat.size());
        if (indices.empty()) {
            indices.resize(mat.size());
            for (std::size_t i = 0; i < mat.size(); ++i) indices[i] = i;
        }
        mat.setLinear(indices, rhsMat);
    } else if (stmt.indices().size() == 2) {
        auto rowIdx = resolveIndexExpr(*stmt.indices()[0], mat.rows());
        auto colIdx = resolveIndexExpr(*stmt.indices()[1], mat.cols());
        mat.setSubmatrix(rowIdx, colIdx, rhsMat);
    } else {
        throw std::runtime_error("Only 1D and 2D matrix indexing is currently supported.");
    }

    workspace_->set(name, mat);
    if (!stmt.suppressOutput) {
        out << name << " =\n\n" << mat.toString() << "\n\n";
    }
}

void Interpreter::executeFor(const ForStmt& stmt, std::ostringstream& out) {
    matforge::core::Value rangeVal = evaluate(stmt.rangeExpr());

    if (rangeVal.isMatrix()) {
        const auto& m = rangeVal.toMatrix();
        for (std::size_t c = 0; c < m.cols(); ++c) {
            if (m.rows() == 1) {
                workspace_->set(stmt.variableName(), matforge::core::Value(m(0, c)));
            } else {
                workspace_->set(stmt.variableName(), matforge::core::Value(m.submatrix({}, {c})));
            }

            try {
                for (const auto& s : stmt.body()) {
                    executeStatement(*s, out);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
    } else if (rangeVal.isDouble()) {
        workspace_->set(stmt.variableName(), rangeVal);
        try {
            for (const auto& s : stmt.body()) {
                executeStatement(*s, out);
            }
        } catch (const BreakException&) {
            // loop broken
        } catch (const ContinueException&) {
            // continued
        }
    }
}

void Interpreter::executeWhile(const WhileStmt& stmt, std::ostringstream& out) {
    while (evaluate(stmt.condition()).toBool()) {
        try {
            for (const auto& s : stmt.body()) {
                executeStatement(*s, out);
            }
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            continue;
        }
    }
}

void Interpreter::executeIf(const IfStmt& stmt, std::ostringstream& out) {
    bool matched = false;
    for (const auto& branch : stmt.branches()) {
        if (evaluate(*branch.condition).toBool()) {
            for (const auto& s : branch.body) {
                executeStatement(*s, out);
            }
            matched = true;
            break;
        }
    }
    if (!matched) {
        for (const auto& s : stmt.elseBody()) {
            executeStatement(*s, out);
        }
    }
}

void Interpreter::executeFunctionDef(const FunctionDefStmt& stmt, std::ostringstream& /* out */) {
    auto funcPtr = std::make_shared<FunctionDefStmt>(
        stmt.functionName(),
        stmt.returnParams(),
        stmt.paramNames(),
        stmt.body()
    );
    workspace_->defineFunction(stmt.functionName(), std::move(funcPtr));
}

std::vector<std::size_t> Interpreter::resolveIndexExpr(const Expr& expr, std::size_t dimSize) {
    if (dynamic_cast<const ColonAllExpr*>(&expr)) {
        return {}; // Empty vector signals all elements along this dimension
    }

    endContextStack_.push_back(dimSize);
    matforge::core::Value val;
    try {
        val = evaluate(expr);
    } catch (...) {
        endContextStack_.pop_back();
        throw;
    }
    endContextStack_.pop_back();

    std::vector<std::size_t> indices;
    if (val.isDouble()) {
        double d = val.toDouble();
        if (d < 1.0) {
            throw std::out_of_range("Subscript indices must either be real positive integers or logicals.");
        }
        indices.push_back(static_cast<std::size_t>(d) - 1);
    } else if (val.isMatrix()) {
        const auto& m = val.toMatrix();
        indices.reserve(m.size());
        for (double d : m.rawData()) {
            if (d < 1.0) {
                throw std::out_of_range("Subscript indices must either be real positive integers or logicals.");
            }
            indices.push_back(static_cast<std::size_t>(d) - 1);
        }
    } else {
        throw std::invalid_argument("Subscript index must be a numeric value or matrix.");
    }

    return indices;
}

matforge::core::Value Interpreter::evaluate(const Expr& expr) {
    if (const auto* num = dynamic_cast<const NumberExpr*>(&expr)) {
        return evaluateNumber(*num);
    }
    if (const auto* str = dynamic_cast<const StringExpr*>(&expr)) {
        return evaluateString(*str);
    }
    if (const auto* var = dynamic_cast<const VariableExpr*>(&expr)) {
        return evaluateVariable(*var);
    }
    if (const auto* rng = dynamic_cast<const RangeExpr*>(&expr)) {
        return evaluateRange(*rng);
    }
    if (const auto* endExp = dynamic_cast<const EndExpr*>(&expr)) {
        return evaluateEnd(*endExp);
    }
    if (dynamic_cast<const ColonAllExpr*>(&expr)) {
        return matforge::core::Value();
    }
    if (const auto* mat = dynamic_cast<const MatrixLiteralExpr*>(&expr)) {
        return evaluateMatrixLiteral(*mat);
    }
    if (const auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        return evaluateBinary(*bin);
    }
    if (const auto* un = dynamic_cast<const UnaryExpr*>(&expr)) {
        return evaluateUnary(*un);
    }
    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        return evaluateCall(*call);
    }
    throw std::runtime_error("Unknown expression node in evaluation.");
}

matforge::core::Value Interpreter::evaluateNumber(const NumberExpr& expr) {
    return matforge::core::Value(expr.value());
}

matforge::core::Value Interpreter::evaluateString(const StringExpr& expr) {
    return matforge::core::Value(expr.value());
}

matforge::core::Value Interpreter::evaluateVariable(const VariableExpr& expr) {
    return workspace_->get(expr.name());
}

matforge::core::Value Interpreter::evaluateRange(const RangeExpr& expr) {
    matforge::core::Value startVal = evaluate(expr.start());
    double start = startVal.toDouble();
    double stop = evaluate(expr.stop()).toDouble();

    double step = 1.0;
    if (expr.hasStep()) {
        step = evaluate(*expr.step()).toDouble();
    } else {
        step = (start <= stop) ? 1.0 : -1.0;
    }

    return matforge::core::Value(matforge::core::Matrix::colon(start, step, stop));
}

matforge::core::Value Interpreter::evaluateEnd(const EndExpr& /* expr */) {
    if (endContextStack_.empty()) {
        throw std::runtime_error("The keyword 'end' can only be used within an indexing expression.");
    }
    return matforge::core::Value(static_cast<double>(endContextStack_.back()));
}

matforge::core::Value Interpreter::evaluateMatrixLiteral(const MatrixLiteralExpr& expr) {
    const auto& rows = expr.rows();
    if (rows.empty()) {
        return matforge::core::Value(matforge::core::Matrix());
    }

    std::size_t numRows = rows.size();
    std::size_t numCols = 0;
    std::vector<double> flattened;

    for (std::size_t r = 0; r < numRows; ++r) {
        const auto& row = rows[r];
        std::size_t colsInThisRow = 0;

        for (const auto& item : row) {
            matforge::core::Value val = evaluate(*item);
            if (val.isDouble()) {
                flattened.push_back(val.toDouble());
                colsInThisRow++;
            } else if (val.isMatrix()) {
                const auto& m = val.toMatrix();
                for (double d : m.rawData()) {
                    flattened.push_back(d);
                }
                colsInThisRow += m.size();
            } else {
                throw std::runtime_error("Matrix literal can only contain numeric values.");
            }
        }

        if (r == 0) {
            numCols = colsInThisRow;
        } else if (colsInThisRow != numCols) {
            throw std::runtime_error("Dimensions of arrays being concatenated are not consistent.");
        }
    }

    return matforge::core::Value(matforge::core::Matrix(numRows, numCols, flattened));
}

matforge::core::Value Interpreter::evaluateBinary(const BinaryExpr& expr) {
    matforge::core::Value left = evaluate(expr.left());
    matforge::core::Value right = evaluate(expr.right());

    switch (expr.op()) {
        case BinaryOp::Add:
            return left + right;
        case BinaryOp::Subtract:
            return left - right;
        case BinaryOp::Multiply:
            return left * right;
        case BinaryOp::Divide:
            if (left.isMatrix() && right.isMatrix()) {
                return matforge::core::Value(left.toMatrix().rightSolve(right.toMatrix()));
            }
            return left / right;
        case BinaryOp::Backslash:
            if (left.isMatrix() && right.isMatrix()) {
                return matforge::core::Value(left.toMatrix().solve(right.toMatrix()));
            }
            if (left.isDouble() && right.isMatrix()) {
                return matforge::core::Value(right.toMatrix() / left.toDouble());
            }
            if (left.isDouble() && right.isDouble()) {
                if (left.toDouble() == 0.0) throw std::runtime_error("Division by zero.");
                return matforge::core::Value(right.toDouble() / left.toDouble());
            }
            throw std::runtime_error("Unsupported operands for backslash (\\).");
        case BinaryOp::DotMultiply:
            if (left.isMatrix() && right.isMatrix()) {
                return matforge::core::Value(left.toMatrix().elementwiseMultiply(right.toMatrix()));
            }
            return left * right;
        case BinaryOp::DotDivide:
            if (left.isMatrix() && right.isMatrix()) {
                return matforge::core::Value(left.toMatrix().elementwiseDivide(right.toMatrix()));
            }
            return left / right;
        case BinaryOp::DotBackslash:
            if (left.isMatrix() && right.isMatrix()) {
                return matforge::core::Value(right.toMatrix().elementwiseDivide(left.toMatrix()));
            }
            if (left.isDouble() && right.isMatrix()) {
                return matforge::core::Value(right.toMatrix() / left.toDouble());
            }
            if (left.isDouble() && right.isDouble()) {
                if (left.toDouble() == 0.0) throw std::runtime_error("Division by zero.");
                return matforge::core::Value(right.toDouble() / left.toDouble());
            }
            throw std::runtime_error("Unsupported operands for dot-backslash (.\\).");
        case BinaryOp::Power:
        case BinaryOp::DotPower:
            if (left.isMatrix() && right.isDouble()) {
                return matforge::core::Value(left.toMatrix().elementwisePower(right.toDouble()));
            }
            if (left.isDouble() && right.isDouble()) {
                return matforge::core::Value(std::pow(left.toDouble(), right.toDouble()));
            }
            throw std::runtime_error("Unsupported operands for power operation.");
        case BinaryOp::Equal:
            return matforge::core::Value(left == right);
        case BinaryOp::NotEqual:
            return matforge::core::Value(left != right);
        case BinaryOp::Less:
            return matforge::core::Value(left.toDouble() < right.toDouble());
        case BinaryOp::LessEqual:
            return matforge::core::Value(left.toDouble() <= right.toDouble());
        case BinaryOp::Greater:
            return matforge::core::Value(left.toDouble() > right.toDouble());
        case BinaryOp::GreaterEqual:
            return matforge::core::Value(left.toDouble() >= right.toDouble());
    }
    throw std::runtime_error("Unknown binary operator.");
}

matforge::core::Value Interpreter::evaluateUnary(const UnaryExpr& expr) {
    matforge::core::Value operand = evaluate(expr.operand());

    switch (expr.op()) {
        case UnaryOp::Plus:
            return operand;
        case UnaryOp::Negate:
            if (operand.isDouble()) {
                return matforge::core::Value(-operand.toDouble());
            }
            if (operand.isMatrix()) {
                return matforge::core::Value(operand.toMatrix() * -1.0);
            }
            throw std::runtime_error("Cannot negate non-numeric value.");
        case UnaryOp::Transpose:
            if (operand.isMatrix()) {
                return matforge::core::Value(operand.toMatrix().transpose());
            }
            if (operand.isDouble()) {
                return operand;
            }
            throw std::runtime_error("Cannot transpose non-matrix value.");
    }
    throw std::runtime_error("Unknown unary operator.");
}

matforge::core::Value Interpreter::evaluateCall(const CallExpr& expr) {
    const std::string& name = expr.callee();

    // 1. Array Indexing on existing workspace variables
    if (workspace_->has(name)) {
        const auto& existingVal = workspace_->get(name);
        if (existingVal.isNumeric()) {
            matforge::core::Matrix mat = existingVal.toMatrix();

            if (expr.args().size() == 1) {
                // 1D Linear indexing: A(idx) or A(:)
                auto indices = resolveIndexExpr(*expr.args()[0], mat.size());
                if (indices.empty()) {
                    // A(:) turns into column vector
                    std::vector<std::size_t> allIndices(mat.size());
                    for (std::size_t i = 0; i < mat.size(); ++i) allIndices[i] = i;
                    return matforge::core::Value(mat.linearSubmatrix(allIndices));
                }
                if (indices.size() == 1) {
                    return matforge::core::Value(mat.rawData()[indices[0]]);
                }
                return matforge::core::Value(mat.linearSubmatrix(indices));
            }

            if (expr.args().size() == 2) {
                // 2D Slicing: A(rows, cols)
                auto rowIdx = resolveIndexExpr(*expr.args()[0], mat.rows());
                auto colIdx = resolveIndexExpr(*expr.args()[1], mat.cols());
                matforge::core::Matrix sub = mat.submatrix(rowIdx, colIdx);

                if (sub.isScalar()) {
                    return matforge::core::Value(sub(0, 0));
                }
                return matforge::core::Value(sub);
            }

            throw std::runtime_error("Only 1D and 2D matrix indexing is currently supported.");
        }
    }

    // 2. User-Defined Functions
    if (workspace_->hasFunction(name)) {
        auto func = workspace_->getFunction(name);
        auto localWs = std::make_shared<Workspace>();

        // Copy existing defined functions to local workspace
        for (const auto& [fName, fDef] : workspace_->functions()) {
            localWs->defineFunction(fName, fDef);
        }

        // Evaluate argument expressions and bind to parameters
        for (std::size_t i = 0; i < func->paramNames().size(); ++i) {
            if (i < expr.args().size()) {
                localWs->set(func->paramNames()[i], evaluate(*expr.args()[i]));
            } else {
                localWs->set(func->paramNames()[i], matforge::core::Value());
            }
        }

        Interpreter localInterp(localWs);
        localInterp.setPlotCallback(plotCallback_);
        localInterp.setHoldCallback(holdCallback_);
        localInterp.setGridCallback(gridCallback_);
        localInterp.setTitleCallback(titleCallback_);
        localInterp.setLabelCallback(labelCallback_);
        localInterp.setLegendCallback(legendCallback_);
        localInterp.setSubplotCallback(subplotCallback_);
        localInterp.setClearScreenCallback(clearScreenCallback_);
        localInterp.setCloseFigureCallback(closeFigureCallback_);
        localInterp.setClearFigureCallback(clearFigureCallback_);

        std::ostringstream localOut;
        matforge::core::Value retVal;
        try {
            for (const auto& s : func->body()) {
                localInterp.executeStatement(*s, localOut);
            }
        } catch (const ReturnException& retEx) {
            retVal = retEx.value;
        }

        if (!retVal.isEmpty()) {
            return retVal;
        }
        if (!func->returnParams().empty() && localWs->has(func->returnParams()[0])) {
            return localWs->get(func->returnParams()[0]);
        }
        return matforge::core::Value();
    }

    // Evaluate arguments for built-in functions
    std::vector<matforge::core::Value> argValues;
    for (const auto& arg : expr.args()) {
        argValues.push_back(evaluate(*arg));
    }

    // 3. Built-in Mathematical Functions
    if (name == "sin") {
        if (argValues.size() != 1) throw std::runtime_error("sin requires 1 argument.");
        return matforge::core::math::sin(argValues[0]);
    }
    if (name == "cos") {
        if (argValues.size() != 1) throw std::runtime_error("cos requires 1 argument.");
        return matforge::core::math::cos(argValues[0]);
    }
    if (name == "tan") {
        if (argValues.size() != 1) throw std::runtime_error("tan requires 1 argument.");
        return matforge::core::math::tan(argValues[0]);
    }
    if (name == "sqrt") {
        if (argValues.size() != 1) throw std::runtime_error("sqrt requires 1 argument.");
        return matforge::core::math::sqrt(argValues[0]);
    }
    if (name == "exp") {
        if (argValues.size() != 1) throw std::runtime_error("exp requires 1 argument.");
        return matforge::core::math::exp(argValues[0]);
    }
    if (name == "log") {
        if (argValues.size() != 1) throw std::runtime_error("log requires 1 argument.");
        return matforge::core::math::log(argValues[0]);
    }
    if (name == "abs") {
        if (argValues.size() != 1) throw std::runtime_error("abs requires 1 argument.");
        if (argValues[0].isMatrix()) {
            const auto& m = argValues[0].toMatrix();
            if (m.cols() == 2 && m.rows() > 0) {
                matforge::core::Matrix mag(m.rows(), 1);
                for (std::size_t i = 0; i < m.rows(); ++i) {
                    double r = m(i, 0);
                    double im = m(i, 1);
                    mag(i, 0) = std::sqrt(r * r + im * im);
                }
                return matforge::core::Value(mag);
            }
        }
        return matforge::core::math::abs(argValues[0]);
    }

    // Matrix Generators
    if (name == "zeros") {
        if (argValues.size() == 1) {
            std::size_t n = static_cast<std::size_t>(argValues[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::zeros(n, n));
        }
        if (argValues.size() == 2) {
            std::size_t r = static_cast<std::size_t>(argValues[0].toDouble());
            std::size_t c = static_cast<std::size_t>(argValues[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::zeros(r, c));
        }
        throw std::runtime_error("zeros requires 1 or 2 arguments.");
    }
    if (name == "ones") {
        if (argValues.size() == 1) {
            std::size_t n = static_cast<std::size_t>(argValues[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::ones(n, n));
        }
        if (argValues.size() == 2) {
            std::size_t r = static_cast<std::size_t>(argValues[0].toDouble());
            std::size_t c = static_cast<std::size_t>(argValues[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::ones(r, c));
        }
        throw std::runtime_error("ones requires 1 or 2 arguments.");
    }
    if (name == "eye") {
        if (argValues.size() == 1) {
            std::size_t n = static_cast<std::size_t>(argValues[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::eye(n));
        }
        if (argValues.size() == 2) {
            std::size_t r = static_cast<std::size_t>(argValues[0].toDouble());
            std::size_t c = static_cast<std::size_t>(argValues[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::eye(r, c));
        }
        throw std::runtime_error("eye requires 1 or 2 arguments.");
    }
    if (name == "linspace") {
        if (argValues.size() == 2) {
            return matforge::core::Value(matforge::core::Matrix::linspace(argValues[0].toDouble(), argValues[1].toDouble()));
        }
        if (argValues.size() == 3) {
            return matforge::core::Value(matforge::core::Matrix::linspace(
                argValues[0].toDouble(), argValues[1].toDouble(), static_cast<std::size_t>(argValues[2].toDouble())));
        }
        throw std::runtime_error("linspace requires 2 or 3 arguments.");
    }

    // Matrix Analysis & Linear Algebra
    if (name == "det") {
        if (argValues.size() != 1) throw std::runtime_error("det requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().determinant());
    }
    if (name == "trace") {
        if (argValues.size() != 1) throw std::runtime_error("trace requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().trace());
    }
    if (name == "sum") {
        if (argValues.size() != 1) throw std::runtime_error("sum requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().sum());
    }
    if (name == "mean") {
        if (argValues.size() != 1) throw std::runtime_error("mean requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().mean());
    }
    if (name == "min") {
        if (argValues.size() != 1) throw std::runtime_error("min requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().min());
    }
    if (name == "max") {
        if (argValues.size() != 1) throw std::runtime_error("max requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().max());
    }
    if (name == "length") {
        if (argValues.size() != 1) throw std::runtime_error("length requires 1 argument.");
        if (argValues[0].isMatrix()) {
            const auto& m = argValues[0].toMatrix();
            return matforge::core::Value(static_cast<double>(std::max(m.rows(), m.cols())));
        }
        return matforge::core::Value(1.0);
    }
    if (name == "size") {
        if (argValues.size() != 1) throw std::runtime_error("size requires 1 argument.");
        if (argValues[0].isMatrix()) {
            const auto& m = argValues[0].toMatrix();
            return matforge::core::Value(matforge::core::Matrix(1, 2, {static_cast<double>(m.rows()), static_cast<double>(m.cols())}));
        }
        return matforge::core::Value(matforge::core::Matrix(1, 2, {1.0, 1.0}));
    }

    // Linear Algebra (Phase 2 - Powered by Eigen 5)
    if (name == "inv") {
        if (argValues.size() != 1) throw std::runtime_error("inv requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().inverse());
    }
    if (name == "pinv") {
        if (argValues.size() == 1) return matforge::core::Value(argValues[0].toMatrix().pinv());
        if (argValues.size() == 2) return matforge::core::Value(argValues[0].toMatrix().pinv(argValues[1].toDouble()));
        throw std::runtime_error("pinv requires 1 or 2 arguments.");
    }
    if (name == "rank") {
        if (argValues.size() == 1) return matforge::core::Value(static_cast<double>(argValues[0].toMatrix().rank()));
        if (argValues.size() == 2) return matforge::core::Value(static_cast<double>(argValues[0].toMatrix().rank(argValues[1].toDouble())));
        throw std::runtime_error("rank requires 1 or 2 arguments.");
    }
    if (name == "cond") {
        if (argValues.size() != 1) throw std::runtime_error("cond requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().cond());
    }
    if (name == "eig") {
        if (argValues.size() != 1) throw std::runtime_error("eig requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().eigenvalues());
    }
    if (name == "svd") {
        if (argValues.size() != 1) throw std::runtime_error("svd requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().singularValues());
    }
    if (name == "chol") {
        if (argValues.size() != 1) throw std::runtime_error("chol requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().chol());
    }
    if (name == "lu") {
        if (argValues.size() != 1) throw std::runtime_error("lu requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().lu().first);
    }
    if (name == "qr") {
        if (argValues.size() != 1) throw std::runtime_error("qr requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().qr().first);
    }

    // Random Number & Matrix Generation (Phase 2)
    if (name == "rand") {
        if (argValues.empty()) return matforge::core::Value(matforge::core::Matrix::rand(1, 1)(0, 0));
        if (argValues.size() == 1) {
            std::size_t n = static_cast<std::size_t>(argValues[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::rand(n, n));
        }
        if (argValues.size() == 2) {
            std::size_t r = static_cast<std::size_t>(argValues[0].toDouble());
            std::size_t c = static_cast<std::size_t>(argValues[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::rand(r, c));
        }
        throw std::runtime_error("rand requires 0, 1, or 2 arguments.");
    }
    if (name == "randn") {
        if (argValues.empty()) return matforge::core::Value(matforge::core::Matrix::randn(1, 1)(0, 0));
        if (argValues.size() == 1) {
            std::size_t n = static_cast<std::size_t>(argValues[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randn(n, n));
        }
        if (argValues.size() == 2) {
            std::size_t r = static_cast<std::size_t>(argValues[0].toDouble());
            std::size_t c = static_cast<std::size_t>(argValues[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randn(r, c));
        }
        throw std::runtime_error("randn requires 0, 1, or 2 arguments.");
    }
    if (name == "randi") {
        if (argValues.empty()) throw std::runtime_error("randi requires at least 1 argument (imax).");
        int imax = static_cast<int>(argValues[0].toDouble());
        if (argValues.size() == 1) {
            return matforge::core::Value(matforge::core::Matrix::randi(imax, 1, 1)(0, 0));
        }
        if (argValues.size() == 2) {
            std::size_t n = static_cast<std::size_t>(argValues[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randi(imax, n, n));
        }
        if (argValues.size() == 3) {
            std::size_t r = static_cast<std::size_t>(argValues[1].toDouble());
            std::size_t c = static_cast<std::size_t>(argValues[2].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randi(imax, r, c));
        }
        throw std::runtime_error("randi requires 1, 2, or 3 arguments.");
    }
    if (name == "rng") {
        if (argValues.size() != 1) throw std::runtime_error("rng requires 1 argument (seed).");
        matforge::core::Matrix::setSeed(static_cast<unsigned int>(argValues[0].toDouble()));
        return matforge::core::Value();
    }

    // Statistics (Phase 2)
    if (name == "std") {
        if (argValues.size() != 1) throw std::runtime_error("std requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().stdDev());
    }
    if (name == "var") {
        if (argValues.size() != 1) throw std::runtime_error("var requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().variance());
    }
    if (name == "median") {
        if (argValues.size() != 1) throw std::runtime_error("median requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().median());
    }

    // Signal Processing & Transforms (Phase 2)
    if (name == "fft") {
        if (argValues.size() != 1) throw std::runtime_error("fft requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().fft());
    }
    if (name == "ifft") {
        if (argValues.size() != 1) throw std::runtime_error("ifft requires 1 argument.");
        return matforge::core::Value(argValues[0].toMatrix().ifft());
    }
    if (name == "conv") {
        if (argValues.size() != 2) throw std::runtime_error("conv requires 2 arguments.");
        return matforge::core::Value(matforge::core::Matrix::conv(argValues[0].toMatrix(), argValues[1].toMatrix()));
    }
    if (name == "real") {
        if (argValues.size() != 1) throw std::runtime_error("real requires 1 argument.");
        if (argValues[0].isMatrix()) {
            const auto& m = argValues[0].toMatrix();
            if (m.cols() == 2 && m.rows() > 0) {
                return matforge::core::Value(m.submatrix({}, {0}));
            }
            return argValues[0];
        }
        return argValues[0];
    }
    if (name == "imag") {
        if (argValues.size() != 1) throw std::runtime_error("imag requires 1 argument.");
        if (argValues[0].isMatrix()) {
            const auto& m = argValues[0].toMatrix();
            if (m.cols() == 2 && m.rows() > 0) {
                return matforge::core::Value(m.submatrix({}, {1}));
            }
            return matforge::core::Value(matforge::core::Matrix::zeros(m.rows(), m.cols()));
        }
        return matforge::core::Value(0.0);
    }

    // File I/O (Phase 2)
    if (name == "readmatrix" || name == "csvread") {
        if (argValues.size() != 1) throw std::runtime_error(name + " requires a file path string.");
        std::string path = argValues[0].isString() ? argValues[0].toStringValue() : "";
        return matforge::core::Value(matforge::core::Matrix::readCSV(path));
    }
    if (name == "writematrix") {
        if (argValues.size() != 2) throw std::runtime_error("writematrix requires 2 arguments: (matrix, filename).");
        std::string path = argValues[1].isString() ? argValues[1].toStringValue() : "";
        argValues[0].toMatrix().writeCSV(path);
        return matforge::core::Value();
    }
    if (name == "csvwrite") {
        if (argValues.size() != 2) throw std::runtime_error("csvwrite requires 2 arguments: (filename, matrix).");
        std::string path = argValues[0].isString() ? argValues[0].toStringValue() : "";
        argValues[1].toMatrix().writeCSV(path);
        return matforge::core::Value();
    }

    // Display & Graphics (Phase 3)
    if (name == "disp") {
        if (argValues.size() != 1) throw std::runtime_error("disp requires 1 argument.");
        return argValues[0];
    }
    if (name == "plot" || name == "bar" || name == "stem") {
        if (plotCallback_ && !argValues.empty()) {
            std::vector<double> xData;
            std::vector<double> yData;
            std::string fmt;

            if (argValues.size() == 1 && argValues[0].isNumeric()) {
                auto ym = argValues[0].toMatrix();
                yData = ym.rawData();
                xData.resize(yData.size());
                for (std::size_t i = 0; i < xData.size(); ++i) xData[i] = static_cast<double>(i + 1);
            } else if (argValues.size() == 2) {
                if (argValues[0].isNumeric() && argValues[1].isNumeric()) {
                    xData = argValues[0].toMatrix().rawData();
                    yData = argValues[1].toMatrix().rawData();
                } else if (argValues[0].isNumeric() && argValues[1].isString()) {
                    auto ym = argValues[0].toMatrix();
                    yData = ym.rawData();
                    xData.resize(yData.size());
                    for (std::size_t i = 0; i < xData.size(); ++i) xData[i] = static_cast<double>(i + 1);
                    fmt = argValues[1].toStringValue();
                }
            } else if (argValues.size() >= 3 && argValues[0].isNumeric() && argValues[1].isNumeric() && argValues[2].isString()) {
                xData = argValues[0].toMatrix().rawData();
                yData = argValues[1].toMatrix().rawData();
                fmt = argValues[2].toStringValue();
            }

            if (!yData.empty()) {
                plotCallback_({xData, yData, fmt, name, ""});
            }
        }
        return matforge::core::Value();
    }
    if (name == "subplot") {
        if (argValues.size() == 3) {
            int r = static_cast<int>(argValues[0].toDouble());
            int c = static_cast<int>(argValues[1].toDouble());
            int p = static_cast<int>(argValues[2].toDouble());
            if (subplotCallback_) subplotCallback_(r, c, p);
        } else if (argValues.size() == 1 && argValues[0].toDouble() >= 100) {
            int val = static_cast<int>(argValues[0].toDouble());
            int r = val / 100;
            int c = (val / 10) % 10;
            int p = val % 10;
            if (subplotCallback_) subplotCallback_(r, c, p);
        }
        return matforge::core::Value();
    }
    if (name == "hold") {
        bool h = true;
        if (!argValues.empty() && argValues[0].isString()) {
            std::string s = argValues[0].toStringValue();
            if (s == "off") h = false;
        }
        if (holdCallback_) holdCallback_(h);
        return matforge::core::Value();
    }
    if (name == "grid") {
        bool g = true;
        if (!argValues.empty() && argValues[0].isString()) {
            std::string s = argValues[0].toStringValue();
            if (s == "off") g = false;
        }
        if (gridCallback_) gridCallback_(g);
        return matforge::core::Value();
    }
    if (name == "title") {
        if (!argValues.empty()) {
            std::string t = argValues[0].isString() ? argValues[0].toStringValue() : argValues[0].formatOutput();
            if (titleCallback_) titleCallback_(t);
        }
        return matforge::core::Value();
    }
    if (name == "xlabel") {
        if (!argValues.empty()) {
            std::string l = argValues[0].isString() ? argValues[0].toStringValue() : argValues[0].formatOutput();
            if (labelCallback_) labelCallback_(l, true);
        }
        return matforge::core::Value();
    }
    if (name == "ylabel") {
        if (!argValues.empty()) {
            std::string l = argValues[0].isString() ? argValues[0].toStringValue() : argValues[0].formatOutput();
            if (labelCallback_) labelCallback_(l, false);
        }
        return matforge::core::Value();
    }
    if (name == "legend") {
        std::vector<std::string> labels;
        for (const auto& a : argValues) {
            labels.push_back(a.isString() ? a.toStringValue() : a.formatOutput());
        }
        if (legendCallback_) legendCallback_(labels);
        return matforge::core::Value();
    }
    if (name == "close") {
        if (closeFigureCallback_) closeFigureCallback_();
        return matforge::core::Value();
    }
    if (name == "clf") {
        if (clearFigureCallback_) clearFigureCallback_();
        return matforge::core::Value();
    }

    // Script execution: run('file.m')
    if (name == "run") {
        if (argValues.empty()) {
            throw std::runtime_error("run requires a script file path.");
        }
        std::string path = argValues[0].isString() ? argValues[0].toStringValue() : "";
        if (path.empty()) {
            throw std::runtime_error("Invalid file path for run command.");
        }

        std::ifstream file(path);
        if (!file.is_open() && path.find('.') == std::string::npos) {
            file.open(path + ".m");
        }
        if (!file.is_open()) {
            throw std::runtime_error("Could not open script file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto runRes = execute(buffer.str());
        if (!runRes.success) {
            throw std::runtime_error("Error running script: " + runRes.errorMessage);
        }
        return matforge::core::Value(runRes.output);
    }

    throw std::runtime_error("Undefined function or variable '" + name + "'.");
}

} // namespace matforge::engine

#include "matforge/engine/Interpreter.hpp"
#include "matforge/engine/Lexer.hpp"
#include "matforge/engine/Parser.hpp"
#include "matforge/core/MathUtils.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace matforge::engine {

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
                << "Supported commands: clear, clc, who, whos, help, exit, quit\n"
                << "Built-in functions: zeros, ones, eye, linspace, sin, cos, tan, sqrt, exp, log, abs, det, trace, sum, mean, min, max, size, length, disp, plot\n\n";
        } else if (cmd == "exit" || cmd == "quit") {
            std::exit(0);
        }
    }
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
            return left / right;
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

    // Check if callee is a variable in workspace -> Array Indexing (1-based MATLAB convention)
    if (workspace_->has(name) && workspace_->get(name).isMatrix()) {
        const auto& mat = workspace_->get(name).toMatrix();
        if (expr.args().size() == 1) {
            // Linear 1-based indexing
            std::size_t idx = static_cast<std::size_t>(evaluate(*expr.args()[0]).toDouble());
            if (idx == 0 || idx > mat.size()) {
                throw std::out_of_range("Index exceeds matrix dimensions.");
            }
            return matforge::core::Value(mat.rawData()[idx - 1]);
        }
        if (expr.args().size() == 2) {
            std::size_t r = static_cast<std::size_t>(evaluate(*expr.args()[0]).toDouble());
            std::size_t c = static_cast<std::size_t>(evaluate(*expr.args()[1]).toDouble());
            if (r == 0 || r > mat.rows() || c == 0 || c > mat.cols()) {
                throw std::out_of_range("Index in position exceeds array bounds.");
            }
            return matforge::core::Value(mat(r - 1, c - 1));
        }
        throw std::runtime_error("Invalid number of indices for matrix.");
    }

    // Evaluate arguments
    std::vector<matforge::core::Value> argValues;
    for (const auto& arg : expr.args()) {
        argValues.push_back(evaluate(*arg));
    }

    // Built-in Mathematical Functions
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

    // Display & Graphics
    if (name == "disp") {
        if (argValues.size() != 1) throw std::runtime_error("disp requires 1 argument.");
        return argValues[0];
    }
    if (name == "plot") {
        if (plotCallback_) {
            if (argValues.size() == 1 && argValues[0].isMatrix()) {
                const auto& yMat = argValues[0].toMatrix();
                std::vector<double> x(yMat.size());
                for (std::size_t i = 0; i < x.size(); ++i) x[i] = static_cast<double>(i + 1);
                plotCallback_(x, yMat.rawData(), "Figure 1");
                return matforge::core::Value();
            }
            if (argValues.size() == 2 && argValues[0].isMatrix() && argValues[1].isMatrix()) {
                plotCallback_(argValues[0].toMatrix().rawData(), argValues[1].toMatrix().rawData(), "Figure 1");
                return matforge::core::Value();
            }
        }
        return matforge::core::Value();
    }

    throw std::runtime_error("Undefined function or variable '" + name + "'.");
}

} // namespace matforge::engine

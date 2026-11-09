#include "matforge/engine/BuiltinRegistry.hpp"
#include "matforge/engine/Interpreter.hpp"
#include "matforge/core/MathUtils.hpp"
#include "matforge/core/Matrix.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace matforge::engine {

BuiltinRegistry& BuiltinRegistry::instance() {
    static BuiltinRegistry reg;
    return reg;
}

BuiltinRegistry::BuiltinRegistry() {
    registerCoreMath();
    registerMatrixGenerators();
    registerLinearAlgebra();
    registerStatistics();
    registerSignalProcessing();
    registerFileIO();
    registerGraphics();
    registerSolversAndCalculus();
}

bool BuiltinRegistry::has(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

matforge::core::Value BuiltinRegistry::call(const std::string& name,
                                           const std::vector<matforge::core::Value>& args,
                                           Interpreter& interp) const {
    auto it = functions_.find(name);
    if (it == functions_.end()) {
        throw std::runtime_error("Undefined function or variable '" + name + "'.");
    }
    return it->second(args, interp);
}

void BuiltinRegistry::registerFunction(const std::string& name, BuiltinFunction func) {
    functions_[name] = std::move(func);
}

// ----------------------------------------------------------------------------
// Core Elementary Math
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerCoreMath() {
    registerFunction("sin", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("sin requires 1 argument.");
        return matforge::core::math::sin(args[0]);
    });
    registerFunction("cos", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("cos requires 1 argument.");
        return matforge::core::math::cos(args[0]);
    });
    registerFunction("tan", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("tan requires 1 argument.");
        return matforge::core::math::tan(args[0]);
    });
    registerFunction("sqrt", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("sqrt requires 1 argument.");
        return matforge::core::math::sqrt(args[0]);
    });
    registerFunction("exp", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("exp requires 1 argument.");
        return matforge::core::math::exp(args[0]);
    });
    registerFunction("log", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("log requires 1 argument.");
        return matforge::core::math::log(args[0]);
    });
    registerFunction("abs", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("abs requires 1 argument.");
        if (args[0].isMatrix()) {
            const auto& m = args[0].toMatrix();
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
        return matforge::core::math::abs(args[0]);
    });
}

// ----------------------------------------------------------------------------
// Matrix Generators
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerMatrixGenerators() {
    registerFunction("zeros", [](const auto& args, auto&) {
        if (args.size() == 1) {
            auto n = static_cast<std::size_t>(args[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::zeros(n, n));
        }
        if (args.size() == 2) {
            auto r = static_cast<std::size_t>(args[0].toDouble());
            auto c = static_cast<std::size_t>(args[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::zeros(r, c));
        }
        throw std::runtime_error("zeros requires 1 or 2 arguments.");
    });
    registerFunction("ones", [](const auto& args, auto&) {
        if (args.size() == 1) {
            auto n = static_cast<std::size_t>(args[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::ones(n, n));
        }
        if (args.size() == 2) {
            auto r = static_cast<std::size_t>(args[0].toDouble());
            auto c = static_cast<std::size_t>(args[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::ones(r, c));
        }
        throw std::runtime_error("ones requires 1 or 2 arguments.");
    });
    registerFunction("eye", [](const auto& args, auto&) {
        if (args.size() == 1) {
            auto n = static_cast<std::size_t>(args[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::eye(n));
        }
        if (args.size() == 2) {
            auto r = static_cast<std::size_t>(args[0].toDouble());
            auto c = static_cast<std::size_t>(args[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::eye(r, c));
        }
        throw std::runtime_error("eye requires 1 or 2 arguments.");
    });
    registerFunction("linspace", [](const auto& args, auto&) {
        if (args.size() == 2) {
            return matforge::core::Value(matforge::core::Matrix::linspace(args[0].toDouble(), args[1].toDouble()));
        }
        if (args.size() == 3) {
            return matforge::core::Value(matforge::core::Matrix::linspace(
                args[0].toDouble(), args[1].toDouble(), static_cast<std::size_t>(args[2].toDouble())));
        }
        throw std::runtime_error("linspace requires 2 or 3 arguments.");
    });
}

// ----------------------------------------------------------------------------
// Linear Algebra & Matrix Analysis
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerLinearAlgebra() {
    registerFunction("det", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("det requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().determinant());
    });
    registerFunction("trace", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("trace requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().trace());
    });
    registerFunction("length", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("length requires 1 argument.");
        if (args[0].isMatrix()) {
            const auto& m = args[0].toMatrix();
            return matforge::core::Value(static_cast<double>(std::max(m.rows(), m.cols())));
        }
        return matforge::core::Value(1.0);
    });
    registerFunction("size", [](const auto& args, auto&) {
        if (args.empty()) throw std::runtime_error("size requires at least 1 argument.");
        if (!args[0].isMatrix()) {
            if (args.size() == 2) return matforge::core::Value(1.0);
            return matforge::core::Value(matforge::core::Matrix({{1.0, 1.0}}));
        }
        const auto& m = args[0].toMatrix();
        if (args.size() == 2) {
            int dim = static_cast<int>(args[1].toDouble());
            if (dim == 1) return matforge::core::Value(static_cast<double>(m.rows()));
            if (dim == 2) return matforge::core::Value(static_cast<double>(m.cols()));
            return matforge::core::Value(1.0);
        }
        return matforge::core::Value(matforge::core::Matrix({{static_cast<double>(m.rows()), static_cast<double>(m.cols())}}));
    });
    registerFunction("inv", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("inv requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().inverse());
    });
    registerFunction("pinv", [](const auto& args, auto&) {
        if (args.empty() || args.size() > 2) throw std::runtime_error("pinv requires 1 or 2 arguments.");
        double tol = (args.size() == 2) ? args[1].toDouble() : -1.0;
        return matforge::core::Value(args[0].toMatrix().pinv(tol));
    });
    registerFunction("rank", [](const auto& args, auto&) {
        if (args.empty() || args.size() > 2) throw std::runtime_error("rank requires 1 or 2 arguments.");
        double tol = (args.size() == 2) ? args[1].toDouble() : -1.0;
        return matforge::core::Value(static_cast<double>(args[0].toMatrix().rank(tol)));
    });
    registerFunction("cond", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("cond requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().cond());
    });
    registerFunction("svd", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("svd requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().singularValues());
    });
    registerFunction("eig", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("eig requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().eigenvalues());
    });
    registerFunction("lu", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("lu requires 1 argument.");
        auto [L, U] = args[0].toMatrix().lu();
        return matforge::core::Value(U);
    });
    registerFunction("qr", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("qr requires 1 argument.");
        auto [Q, R] = args[0].toMatrix().qr();
        return matforge::core::Value(R);
    });
    registerFunction("chol", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("chol requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().chol());
    });
}

// ----------------------------------------------------------------------------
// Statistics & Random Numbers
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerStatistics() {
    registerFunction("rand", [](const auto& args, auto&) {
        if (args.empty()) return matforge::core::Value(matforge::core::Matrix::rand(1, 1)(0, 0));
        if (args.size() == 1) {
            auto n = static_cast<std::size_t>(args[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::rand(n, n));
        }
        if (args.size() == 2) {
            auto r = static_cast<std::size_t>(args[0].toDouble());
            auto c = static_cast<std::size_t>(args[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::rand(r, c));
        }
        throw std::runtime_error("rand requires 0, 1, or 2 arguments.");
    });
    registerFunction("randn", [](const auto& args, auto&) {
        if (args.empty()) return matforge::core::Value(matforge::core::Matrix::randn(1, 1)(0, 0));
        if (args.size() == 1) {
            auto n = static_cast<std::size_t>(args[0].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randn(n, n));
        }
        if (args.size() == 2) {
            auto r = static_cast<std::size_t>(args[0].toDouble());
            auto c = static_cast<std::size_t>(args[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randn(r, c));
        }
        throw std::runtime_error("randn requires 0, 1, or 2 arguments.");
    });
    registerFunction("randi", [](const auto& args, auto&) {
        if (args.size() < 2 || args.size() > 3) throw std::runtime_error("randi requires 2 or 3 arguments.");
        int imax = static_cast<int>(args[0].toDouble());
        if (args.size() == 2) {
            auto n = static_cast<std::size_t>(args[1].toDouble());
            return matforge::core::Value(matforge::core::Matrix::randi(imax, n, n));
        }
        auto r = static_cast<std::size_t>(args[1].toDouble());
        auto c = static_cast<std::size_t>(args[2].toDouble());
        return matforge::core::Value(matforge::core::Matrix::randi(imax, r, c));
    });
    registerFunction("rng", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("rng requires a seed integer.");
        matforge::core::Matrix::setSeed(static_cast<unsigned int>(args[0].toDouble()));
        return matforge::core::Value();
    });
    registerFunction("std", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("std requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().stdDev());
    });
    registerFunction("var", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("var requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().variance());
    });
    registerFunction("median", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("median requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().median());
    });
    registerFunction("sum", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("sum requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().sum());
    });
    registerFunction("mean", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("mean requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().mean());
    });
    registerFunction("min", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("min requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().min());
    });
    registerFunction("max", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("max requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().max());
    });
}

// ----------------------------------------------------------------------------
// Signal Processing & Transforms
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerSignalProcessing() {
    registerFunction("conv", [](const auto& args, auto&) {
        if (args.size() != 2) throw std::runtime_error("conv requires 2 arguments.");
        return matforge::core::Value(matforge::core::Matrix::conv(args[0].toMatrix(), args[1].toMatrix()));
    });
    registerFunction("fft", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("fft requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().fft());
    });
    registerFunction("ifft", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("ifft requires 1 argument.");
        return matforge::core::Value(args[0].toMatrix().ifft());
    });
    registerFunction("real", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("real requires 1 argument.");
        if (args[0].isMatrix()) {
            const auto& m = args[0].toMatrix();
            if (m.cols() == 2 && m.rows() > 0) {
                return matforge::core::Value(m.submatrix({}, {0}));
            }
            return args[0];
        }
        return args[0];
    });
    registerFunction("imag", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("imag requires 1 argument.");
        if (args[0].isMatrix()) {
            const auto& m = args[0].toMatrix();
            if (m.cols() == 2 && m.rows() > 0) {
                return matforge::core::Value(m.submatrix({}, {1}));
            }
            return matforge::core::Value(matforge::core::Matrix::zeros(m.rows(), m.cols()));
        }
        return matforge::core::Value(0.0);
    });
}

// ----------------------------------------------------------------------------
// File I/O & Scripting
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerFileIO() {
    registerFunction("readmatrix", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("readmatrix requires a file path string.");
        std::string path = args[0].isString() ? args[0].toStringValue() : "";
        return matforge::core::Value(matforge::core::Matrix::readCSV(path));
    });
    registerFunction("csvread", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("csvread requires a file path string.");
        std::string path = args[0].isString() ? args[0].toStringValue() : "";
        return matforge::core::Value(matforge::core::Matrix::readCSV(path));
    });
    registerFunction("writematrix", [](const auto& args, auto&) {
        if (args.size() != 2) throw std::runtime_error("writematrix requires 2 arguments: (matrix, filename).");
        std::string path = args[1].isString() ? args[1].toStringValue() : "";
        args[0].toMatrix().writeCSV(path);
        return matforge::core::Value();
    });
    registerFunction("csvwrite", [](const auto& args, auto&) {
        if (args.size() != 2) throw std::runtime_error("csvwrite requires 2 arguments: (filename, matrix).");
        std::string path = args[0].isString() ? args[0].toStringValue() : "";
        args[1].toMatrix().writeCSV(path);
        return matforge::core::Value();
    });
    registerFunction("run", [](const auto& args, auto& interp) {
        if (args.empty()) throw std::runtime_error("run requires a script file path.");
        std::string path = args[0].isString() ? args[0].toStringValue() : "";
        if (path.empty()) throw std::runtime_error("Invalid file path for run command.");

        std::ifstream file(path);
        if (!file.is_open() && path.find('.') == std::string::npos) {
            file.open(path + ".m");
        }
        if (!file.is_open()) {
            throw std::runtime_error("Could not open script file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto runRes = interp.execute(buffer.str());
        if (!runRes.success) {
            throw std::runtime_error("Error running script: " + runRes.errorMessage);
        }
        return matforge::core::Value(runRes.output);
    });
}

// ----------------------------------------------------------------------------
// Display & Graphics
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerGraphics() {
    registerFunction("disp", [](const auto& args, auto&) {
        if (args.size() != 1) throw std::runtime_error("disp requires 1 argument.");
        return args[0];
    });

    auto handlePlot = [](const std::string& plotType, const auto& args, auto& interp) {
        if (interp.plotCallback() && !args.empty()) {
            std::vector<double> xData;
            std::vector<double> yData;
            std::string fmt;

            if (args.size() == 1 && args[0].isNumeric()) {
                auto ym = args[0].toMatrix();
                yData = ym.rawData();
                xData.resize(yData.size());
                for (std::size_t i = 0; i < yData.size(); ++i) xData[i] = static_cast<double>(i + 1);
            } else if (args.size() >= 2 && args[0].isNumeric() && args[1].isNumeric()) {
                xData = args[0].toMatrix().rawData();
                yData = args[1].toMatrix().rawData();
                if (args.size() >= 3 && args[2].isString()) {
                    fmt = args[2].toStringValue();
                }
            } else if (args.size() >= 2 && args[0].isNumeric() && args[1].isString()) {
                auto ym = args[0].toMatrix();
                yData = ym.rawData();
                xData.resize(yData.size());
                for (std::size_t i = 0; i < yData.size(); ++i) xData[i] = static_cast<double>(i + 1);
                fmt = args[1].toStringValue();
            }

            if (!yData.empty()) {
                interp.plotCallback()({xData, yData, fmt, plotType, ""});
            }
        }
        return matforge::core::Value();
    };

    registerFunction("plot", [handlePlot](const auto& args, auto& interp) {
        return handlePlot("plot", args, interp);
    });
    registerFunction("bar", [handlePlot](const auto& args, auto& interp) {
        return handlePlot("bar", args, interp);
    });
    registerFunction("stem", [handlePlot](const auto& args, auto& interp) {
        return handlePlot("stem", args, interp);
    });

    registerFunction("subplot", [](const auto& args, auto& interp) {
        if (args.size() == 3) {
            int r = static_cast<int>(args[0].toDouble());
            int c = static_cast<int>(args[1].toDouble());
            int p = static_cast<int>(args[2].toDouble());
            if (interp.subplotCallback()) interp.subplotCallback()(r, c, p);
        } else if (args.size() == 1 && args[0].toDouble() >= 100) {
            int val = static_cast<int>(args[0].toDouble());
            int r = val / 100;
            int c = (val / 10) % 10;
            int p = val % 10;
            if (interp.subplotCallback()) interp.subplotCallback()(r, c, p);
        }
        return matforge::core::Value();
    });

    registerFunction("hold", [](const auto& args, auto& interp) {
        bool h = true;
        if (!args.empty() && args[0].isString()) {
            if (args[0].toStringValue() == "off") h = false;
        }
        if (interp.holdCallback()) interp.holdCallback()(h);
        return matforge::core::Value();
    });

    registerFunction("grid", [](const auto& args, auto& interp) {
        bool g = true;
        if (!args.empty() && args[0].isString()) {
            if (args[0].toStringValue() == "off") g = false;
        }
        if (interp.gridCallback()) interp.gridCallback()(g);
        return matforge::core::Value();
    });

    registerFunction("title", [](const auto& args, auto& interp) {
        if (!args.empty()) {
            std::string t = args[0].isString() ? args[0].toStringValue() : args[0].formatOutput();
            if (interp.titleCallback()) interp.titleCallback()(t);
        }
        return matforge::core::Value();
    });

    registerFunction("xlabel", [](const auto& args, auto& interp) {
        if (!args.empty()) {
            std::string l = args[0].isString() ? args[0].toStringValue() : args[0].formatOutput();
            if (interp.labelCallback()) interp.labelCallback()(l, true);
        }
        return matforge::core::Value();
    });

    registerFunction("ylabel", [](const auto& args, auto& interp) {
        if (!args.empty()) {
            std::string l = args[0].isString() ? args[0].toStringValue() : args[0].formatOutput();
            if (interp.labelCallback()) interp.labelCallback()(l, false);
        }
        return matforge::core::Value();
    });

    registerFunction("legend", [](const auto& args, auto& interp) {
        std::vector<std::string> labels;
        for (const auto& a : args) {
            labels.push_back(a.isString() ? a.toStringValue() : a.formatOutput());
        }
        if (interp.legendCallback()) interp.legendCallback()(labels);
        return matforge::core::Value();
    });

    registerFunction("close", [](const auto&, auto& interp) {
        if (interp.closeFigureCallback()) interp.closeFigureCallback()();
        return matforge::core::Value();
    });

    registerFunction("clf", [](const auto&, auto& interp) {
        if (interp.clearFigureCallback()) interp.clearFigureCallback()();
        return matforge::core::Value();
    });
}

// ----------------------------------------------------------------------------
// Solvers & Numerical Calculus
// ----------------------------------------------------------------------------
void BuiltinRegistry::registerSolversAndCalculus() {
    registerFunction("solve", [](const auto& args, auto&) {
        if (args.size() == 2) {
            return matforge::core::math::solve(args[0], args[1]);
        }
        if (args.size() == 1) {
            return matforge::core::math::solve(args[0], matforge::core::Value(0.0));
        }
        throw std::runtime_error("solve requires 1 or 2 arguments: solve(A, b) or solve(@fun, x0).");
    });
    registerFunction("linsolve", [](const auto& args, auto&) {
        if (args.size() != 2) throw std::runtime_error("linsolve requires 2 arguments (A, b).");
        return matforge::core::math::linsolve(args[0], args[1]);
    });
    registerFunction("fzero", [](const auto& args, auto&) {
        if (args.size() < 2) throw std::runtime_error("fzero requires at least 2 arguments (fun, x0).");
        double tol = (args.size() >= 3) ? args[2].toDouble() : 1e-12;
        return matforge::core::Value(matforge::core::math::fzero(args[0], args[1].toDouble(), tol));
    });
    registerFunction("fsolve", [](const auto& args, auto&) {
        if (args.size() < 2) throw std::runtime_error("fsolve requires at least 2 arguments (fun, x0).");
        double tol = (args.size() >= 3) ? args[2].toDouble() : 1e-8;
        int maxIter = (args.size() >= 4) ? static_cast<int>(args[3].toDouble()) : 200;
        return matforge::core::math::fsolve(args[0], args[1], tol, maxIter);
    });
    registerFunction("integral", [](const auto& args, auto&) {
        if (args.size() < 3) throw std::runtime_error("integral requires at least 3 arguments (fun, a, b).");
        double a = args[1].toDouble();
        double b = args[2].toDouble();
        double tol = (args.size() >= 4) ? args[3].toDouble() : 1e-10;
        return matforge::core::Value(matforge::core::math::integral(args[0], a, b, tol));
    });
    registerFunction("trapz", [](const auto& args, auto&) {
        if (args.size() == 1) {
            return matforge::core::Value(matforge::core::math::trapz(args[0].toMatrix()));
        }
        if (args.size() == 2) {
            return matforge::core::Value(matforge::core::math::trapz(args[0].toMatrix(), args[1].toMatrix()));
        }
        throw std::runtime_error("trapz requires 1 or 2 arguments.");
    });
    registerFunction("cumtrapz", [](const auto& args, auto&) {
        if (args.size() == 1) {
            return matforge::core::Value(matforge::core::math::cumtrapz(args[0].toMatrix()));
        }
        if (args.size() == 2) {
            return matforge::core::Value(matforge::core::math::cumtrapz(args[0].toMatrix(), args[1].toMatrix()));
        }
        throw std::runtime_error("cumtrapz requires 1 or 2 arguments.");
    });
    registerFunction("diff", [](const auto& args, auto&) {
        if (args.size() == 1) {
            return matforge::core::Value(matforge::core::math::diff(args[0].toMatrix()));
        }
        if (args.size() == 2) {
            return matforge::core::Value(matforge::core::math::diff(args[0].toMatrix(), static_cast<int>(args[1].toDouble())));
        }
        throw std::runtime_error("diff requires 1 or 2 arguments.");
    });
    registerFunction("gradient", [](const auto& args, auto&) {
        if (args.size() == 1) {
            return matforge::core::Value(matforge::core::math::gradient(args[0].toMatrix()));
        }
        if (args.size() == 2) {
            return matforge::core::Value(matforge::core::math::gradient(args[0].toMatrix(), args[1].toDouble()));
        }
        throw std::runtime_error("gradient requires 1 or 2 arguments.");
    });
    registerFunction("fminsearch", [](const auto& args, auto&) {
        if (args.size() < 2) throw std::runtime_error("fminsearch requires at least 2 arguments (fun, x0).");
        double tol = (args.size() >= 3) ? args[2].toDouble() : 1e-6;
        int maxIter = (args.size() >= 4) ? static_cast<int>(args[3].toDouble()) : 2000;
        return matforge::core::math::fminsearch(args[0], args[1], tol, maxIter);
    });
    registerFunction("ode45", [](const auto& args, auto&) {
        if (args.size() < 3) throw std::runtime_error("ode45 requires at least 3 arguments (odefun, tspan, y0).");
        auto odeRes = matforge::core::math::ode45(args[0], args[1], args[2]);
        matforge::core::Matrix combined(odeRes.t.rows(), odeRes.t.cols() + odeRes.y.cols());
        for (std::size_t i = 0; i < odeRes.t.rows(); ++i) {
            combined(i, 0) = odeRes.t(i, 0);
            for (std::size_t j = 0; j < odeRes.y.cols(); ++j) {
                combined(i, 1 + j) = odeRes.y(i, j);
            }
        }
        return matforge::core::Value(combined);
    });
    registerFunction("ode23", [](const auto& args, auto&) {
        if (args.size() < 3) throw std::runtime_error("ode23 requires at least 3 arguments (odefun, tspan, y0).");
        auto odeRes = matforge::core::math::ode23(args[0], args[1], args[2]);
        matforge::core::Matrix combined(odeRes.t.rows(), odeRes.t.cols() + odeRes.y.cols());
        for (std::size_t i = 0; i < odeRes.t.rows(); ++i) {
            combined(i, 0) = odeRes.t(i, 0);
            for (std::size_t j = 0; j < odeRes.y.cols(); ++j) {
                combined(i, 1 + j) = odeRes.y(i, j);
            }
        }
        return matforge::core::Value(combined);
    });
}

} // namespace matforge::engine

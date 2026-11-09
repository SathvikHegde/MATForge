#include "matforge/core/MathUtils.hpp"

#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <vector>
#include <limits>

namespace matforge::core::math {

// ----------------------------------------------------------------------------
// Elementary functions
// ----------------------------------------------------------------------------

Matrix apply(const Matrix& m, double (*func)(double)) {
    Matrix res(m.rows(), m.cols());
    for (std::size_t i = 0; i < m.size(); ++i) {
        res.rawData()[i] = func(m.rawData()[i]);
    }
    return res;
}

static Value applyFunction(const Value& val, double (*func)(double)) {
    if (val.isDouble()) {
        return Value(func(val.toDouble()));
    }
    if (val.isMatrix()) {
        return Value(apply(val.toMatrix(), func));
    }
    throw std::runtime_error("Mathematical function can only be applied to numeric values.");
}

Value sin(const Value& val) { return applyFunction(val, std::sin); }
Value cos(const Value& val) { return applyFunction(val, std::cos); }
Value tan(const Value& val) { return applyFunction(val, std::tan); }
Value sqrt(const Value& val) { return applyFunction(val, std::sqrt); }
Value exp(const Value& val) { return applyFunction(val, std::exp); }
Value log(const Value& val) { return applyFunction(val, std::log); }
Value abs(const Value& val) { return applyFunction(val, std::abs); }

// ----------------------------------------------------------------------------
// Calculus: Differences & Numerical Gradient
// ----------------------------------------------------------------------------

Matrix diff(const Matrix& x, int n) {
    if (n < 1) return x;
    if (x.size() == 0) return Matrix(0, 0);

    // 1D row vector (1 x N)
    if (x.rows() == 1) {
        if (x.cols() <= 1) return Matrix(1, 0);
        Matrix d(1, x.cols() - 1);
        for (std::size_t i = 0; i < x.cols() - 1; ++i) {
            d(0, i) = x(0, i + 1) - x(0, i);
        }
        return (n == 1) ? d : diff(d, n - 1);
    }

    // 1D column vector (N x 1)
    if (x.cols() == 1) {
        if (x.rows() <= 1) return Matrix(0, 1);
        Matrix d(x.rows() - 1, 1);
        for (std::size_t i = 0; i < x.rows() - 1; ++i) {
            d(i, 0) = x(i + 1, 0) - x(i, 0);
        }
        return (n == 1) ? d : diff(d, n - 1);
    }

    // 2D matrix (R x C): difference along rows
    if (x.rows() <= 1) return Matrix(0, x.cols());
    Matrix d(x.rows() - 1, x.cols());
    for (std::size_t r = 0; r < x.rows() - 1; ++r) {
        for (std::size_t c = 0; c < x.cols(); ++c) {
            d(r, c) = x(r + 1, c) - x(r, c);
        }
    }
    return (n == 1) ? d : diff(d, n - 1);
}

Matrix gradient(const Matrix& y, double h) {
    if (h == 0.0) throw std::runtime_error("gradient: spacing h cannot be zero.");
    if (y.size() == 0) return Matrix(0, 0);
    if (y.size() == 1) return Matrix::zeros(y.rows(), y.cols());

    std::size_t N = y.size();
    Matrix grad(y.rows(), y.cols());

    // 1D vector (row or column)
    if (y.rows() == 1 || y.cols() == 1) {
        const auto& raw = y.rawData();
        auto& res = grad.rawData();
        res[0] = (raw[1] - raw[0]) / h;
        for (std::size_t i = 1; i < N - 1; ++i) {
            res[i] = (raw[i + 1] - raw[i - 1]) / (2.0 * h);
        }
        res[N - 1] = (raw[N - 1] - raw[N - 2]) / h;
        return grad;
    }

    // 2D matrix: gradient along rows (dim 1)
    for (std::size_t c = 0; c < y.cols(); ++c) {
        grad(0, c) = (y(1, c) - y(0, c)) / h;
        for (std::size_t r = 1; r < y.rows() - 1; ++r) {
            grad(r, c) = (y(r + 1, c) - y(r - 1, c)) / (2.0 * h);
        }
        grad(y.rows() - 1, c) = (y(y.rows() - 1, c) - y(y.rows() - 2, c)) / h;
    }
    return grad;
}

// ----------------------------------------------------------------------------
// Trapezoidal Integration (trapz, cumtrapz)
// ----------------------------------------------------------------------------

double trapz(const Matrix& y) {
    if (y.size() < 2) return 0.0;
    const auto& d = y.rawData();
    double s = 0.0;
    for (std::size_t i = 0; i < y.size() - 1; ++i) {
        s += 0.5 * (d[i] + d[i + 1]);
    }
    return s;
}

double trapz(const Matrix& x, const Matrix& y) {
    if (x.size() != y.size()) throw std::runtime_error("trapz: x and y must have the same length.");
    if (x.size() < 2) return 0.0;
    const auto& xd = x.rawData();
    const auto& yd = y.rawData();
    double s = 0.0;
    for (std::size_t i = 0; i < x.size() - 1; ++i) {
        s += 0.5 * (yd[i] + yd[i + 1]) * (xd[i + 1] - xd[i]);
    }
    return s;
}

Matrix cumtrapz(const Matrix& y) {
    Matrix res(y.rows(), y.cols());
    if (y.size() == 0) return res;
    const auto& d = y.rawData();
    auto& r = res.rawData();
    r[0] = 0.0;
    for (std::size_t i = 1; i < y.size(); ++i) {
        r[i] = r[i - 1] + 0.5 * (d[i - 1] + d[i]);
    }
    return res;
}

Matrix cumtrapz(const Matrix& x, const Matrix& y) {
    if (x.size() != y.size()) throw std::runtime_error("cumtrapz: x and y must have the same length.");
    Matrix res(y.rows(), y.cols());
    if (y.size() == 0) return res;
    const auto& xd = x.rawData();
    const auto& yd = y.rawData();
    auto& r = res.rawData();
    r[0] = 0.0;
    for (std::size_t i = 1; i < y.size(); ++i) {
        r[i] = r[i - 1] + 0.5 * (yd[i - 1] + yd[i]) * (xd[i] - xd[i - 1]);
    }
    return res;
}

// ----------------------------------------------------------------------------
// Adaptive Simpson's Quadrature (integral)
// ----------------------------------------------------------------------------

namespace {
double simpsonRecurse(const std::function<double(double)>& f, double a, double b,
                      double fa, double fb, double fc, double S, double tol, int depth) {
    double c = 0.5 * (a + b);
    double h = b - a;
    double d = 0.5 * (a + c);
    double e = 0.5 * (c + b);
    double fd = f(d);
    double fe = f(e);
    double Sleft = (h / 12.0) * (fa + 4.0 * fd + fc);
    double Sright = (h / 12.0) * (fc + 4.0 * fe + fb);
    double S2 = Sleft + Sright;
    double delta = S2 - S;

    if (depth <= 0 || std::abs(delta) <= 15.0 * tol) {
        return S2 + delta / 15.0;
    }
    return simpsonRecurse(f, a, c, fa, fc, fd, Sleft, tol * 0.5, depth - 1) +
           simpsonRecurse(f, c, b, fc, fb, fe, Sright, tol * 0.5, depth - 1);
}
}

double integral(const std::function<double(double)>& f, double a, double b, double tol) {
    if (a == b) return 0.0;
    if (b < a) return -integral(f, b, a, tol);
    double c = 0.5 * (a + b);
    double fa = f(a);
    double fb = f(b);
    double fc = f(c);
    double S = ((b - a) / 6.0) * (fa + 4.0 * fc + fb);
    return simpsonRecurse(f, a, b, fa, fb, fc, S, tol, 40);
}

double integral(const Value& funVal, double a, double b, double tol) {
    if (!funVal.isFunctionHandle()) {
        throw std::runtime_error("integral: first argument must be a function handle.");
    }
    auto f = [&funVal](double x) -> double {
        Value res = funVal.call({Value(x)});
        if (res.isDouble()) return res.toDouble();
        if (res.isMatrix() && res.toMatrix().size() > 0) return res.toMatrix().rawData()[0];
        throw std::runtime_error("Function handle in integral must return a numeric scalar.");
    };
    return integral(f, a, b, tol);
}

// ----------------------------------------------------------------------------
// Root Finding: Brent's Method (fzero)
// ----------------------------------------------------------------------------

double fzero(const std::function<double(double)>& f, double x0, double tol) {
    double a = x0;
    double b = x0;
    double fa = f(a);
    if (std::abs(fa) < 1e-14) return a;

    // Search for a bracketing interval around x0
    double delta = (std::abs(x0) > 0.0) ? std::abs(x0) * 0.05 : 0.05;
    bool foundBracket = false;
    for (int i = 0; i < 60; ++i) {
        a = x0 - delta;
        b = x0 + delta;
        fa = f(a);
        double fb = f(b);
        if (std::abs(fa) < 1e-14) return a;
        if (std::abs(fb) < 1e-14) return b;
        if (fa * fb <= 0.0) {
            foundBracket = true;
            break;
        }
        delta *= 1.6;
    }

    if (!foundBracket) {
        // Fallback: search unilateral bounds
        double step = 0.1;
        for (int i = 0; i < 50; ++i) {
            b = x0 + (i + 1) * step;
            double fb = f(b);
            if (fa * fb <= 0.0) {
                foundBracket = true;
                break;
            }
        }
    }

    if (!foundBracket) {
        throw std::runtime_error("fzero: could not find a sign-changing bracket near initial guess.");
    }

    // Brent-Dekker Root-Finding
    double fb = f(b);
    if (std::abs(fa) < std::abs(fb)) {
        std::swap(a, b);
        std::swap(fa, fb);
    }
    double c = a;
    double fc = fa;
    bool mflag = true;
    double d = 0.0;

    for (int iter = 0; iter < 120; ++iter) {
        if (std::abs(fb) <= tol || std::abs(b - a) <= tol) {
            return b;
        }

        double s = 0.0;
        if (std::abs(fa - fc) > 1e-15 && std::abs(fb - fc) > 1e-15) {
            // Inverse quadratic interpolation
            s = (a * fb * fc) / ((fa - fb) * (fa - fc)) +
                (b * fa * fc) / ((fb - fa) * (fb - fc)) +
                (c * fa * fb) / ((fc - fa) * (fc - fb));
        } else {
            // Secant method
            s = b - fb * (b - a) / (fb - fa);
        }

        bool cond1 = !((s > (3.0 * a + b) / 4.0 && s < b) || (s < (3.0 * a + b) / 4.0 && s > b));
        bool cond2 = mflag && (std::abs(s - b) >= 0.5 * std::abs(b - c));
        bool cond3 = !mflag && (std::abs(s - b) >= 0.5 * std::abs(c - d));
        bool cond4 = mflag && (std::abs(b - c) < tol);
        bool cond5 = !mflag && (std::abs(c - d) < tol);

        if (cond1 || cond2 || cond3 || cond4 || cond5) {
            s = 0.5 * (a + b);
            mflag = true;
        } else {
            mflag = false;
        }

        double fs = f(s);
        d = c;
        c = b;
        fc = fb;

        if (fa * fs < 0.0) {
            b = s;
            fb = fs;
        } else {
            a = s;
            fa = fs;
        }

        if (std::abs(fa) < std::abs(fb)) {
            std::swap(a, b);
            std::swap(fa, fb);
        }
    }

    return b;
}

double fzero(const Value& funVal, double x0, double tol) {
    if (!funVal.isFunctionHandle()) {
        throw std::runtime_error("fzero: first argument must be a function handle.");
    }
    auto f = [&funVal](double x) -> double {
        Value res = funVal.call({Value(x)});
        if (res.isDouble()) return res.toDouble();
        if (res.isMatrix() && res.toMatrix().size() > 0) return res.toMatrix().rawData()[0];
        throw std::runtime_error("Function handle in fzero must return a numeric scalar.");
    };
    return fzero(f, x0, tol);
}

// ----------------------------------------------------------------------------
// Nonlinear Equation Solver (fsolve)
// ----------------------------------------------------------------------------

Matrix fsolve(const std::function<Matrix(const Matrix&)>& F, const Matrix& x0, double tol, int maxIter) {
    Matrix x = x0;
    if (x.rows() == 1 && x.cols() > 1) {
        x = x.transpose();
    }
    std::size_t n = x.size();
    if (n == 0) return x;

    double lambda = 1e-4;

    for (int iter = 0; iter < maxIter; ++iter) {
        Matrix Fx = F(x);
        if (Fx.rows() == 1 && Fx.cols() > 1) {
            Fx = Fx.transpose();
        }
        if (Fx.size() != n) {
            throw std::runtime_error("fsolve: function output dimension does not match input dimension.");
        }

        double maxErr = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            maxErr = std::max(maxErr, std::abs(Fx.rawData()[i]));
        }
        if (maxErr <= tol) {
            return (x0.rows() == 1) ? x.transpose() : x;
        }

        // Numerical Jacobian J (n x n)
        Eigen::MatrixXd J(n, n);
        for (std::size_t j = 0; j < n; ++j) {
            double xj = x.rawData()[j];
            double eps = std::max(std::abs(xj) * 1e-7, 1e-8);
            Matrix xPlus = x;
            xPlus.rawData()[j] += eps;
            Matrix FxPlus = F(xPlus);
            if (FxPlus.rows() == 1 && FxPlus.cols() > 1) FxPlus = FxPlus.transpose();
            for (std::size_t i = 0; i < n; ++i) {
                J(i, j) = (FxPlus.rawData()[i] - Fx.rawData()[i]) / eps;
            }
        }

        Eigen::VectorXd Fvec(n);
        for (std::size_t i = 0; i < n; ++i) Fvec(i) = Fx.rawData()[i];

        // Solve (J^T J + lambda * I) delta = -J^T Fvec (Levenberg-Marquardt)
        Eigen::MatrixXd JT = J.transpose();
        Eigen::MatrixXd A = JT * J + lambda * Eigen::MatrixXd::Identity(n, n);
        Eigen::VectorXd b = -JT * Fvec;
        Eigen::VectorXd delta = A.colPivHouseholderQr().solve(b);

        Matrix xNew = x;
        for (std::size_t i = 0; i < n; ++i) xNew.rawData()[i] += delta(i);
        Matrix FxNew = F(xNew);
        if (FxNew.rows() == 1 && FxNew.cols() > 1) FxNew = FxNew.transpose();

        double newErr = 0.0;
        for (std::size_t i = 0; i < n; ++i) newErr = std::max(newErr, std::abs(FxNew.rawData()[i]));

        if (newErr < maxErr) {
            x = xNew;
            lambda = std::max(lambda * 0.1, 1e-12);
            if (delta.norm() <= tol) {
                return (x0.rows() == 1) ? x.transpose() : x;
            }
        } else {
            lambda = std::min(lambda * 10.0, 1e8);
            for (std::size_t i = 0; i < n; ++i) xNew.rawData()[i] = x.rawData()[i] + 0.5 * delta(i);
            x = xNew;
        }
    }

    return (x0.rows() == 1) ? x.transpose() : x;
}

Value fsolve(const Value& funVal, const Value& x0Val, double tol, int maxIter) {
    if (!funVal.isFunctionHandle()) {
        throw std::runtime_error("fsolve: first argument must be a function handle.");
    }

    bool isScalarInput = x0Val.isDouble();
    Matrix x0 = isScalarInput ? Matrix({{x0Val.toDouble()}}) : x0Val.toMatrix();

    auto F = [&funVal](const Matrix& x) -> Matrix {
        Value res = funVal.call({Value(x)});
        if (res.isDouble()) {
            return Matrix({{res.toDouble()}});
        }
        if (res.isMatrix()) {
            return res.toMatrix();
        }
        throw std::runtime_error("Function in fsolve must return a numeric value.");
    };

    Matrix sol = fsolve(F, x0, tol, maxIter);
    if (isScalarInput) {
        return Value(sol(0, 0));
    }
    return Value(sol);
}

// ----------------------------------------------------------------------------
// Linear Solve & Unified Solve
// ----------------------------------------------------------------------------

Value linsolve(const Value& A, const Value& b) {
    if (A.isDouble() && b.isDouble()) {
        if (std::abs(A.toDouble()) < 1e-15) {
            throw std::runtime_error("linsolve: singular system (division by zero).");
        }
        return Value(b.toDouble() / A.toDouble());
    }

    Matrix matA = A.isMatrix() ? A.toMatrix() : Matrix({{A.toDouble()}});
    Matrix matB = b.isMatrix() ? b.toMatrix() : Matrix({{b.toDouble()}});

    return Value(matA.solve(matB));
}

Value solve(const Value& a, const Value& b) {
    if (a.isFunctionHandle()) {
        return fsolve(a, b);
    }
    if (a.isNumeric() && b.isNumeric()) {
        return linsolve(a, b);
    }
    throw std::runtime_error("solve: arguments must be (A, b) for linear solve or (@fun, x0) for nonlinear solve.");
}

// ----------------------------------------------------------------------------
// Multivariable Optimization: Nelder-Mead Simplex (fminsearch)
// ----------------------------------------------------------------------------

Matrix fminsearch(const std::function<double(const Matrix&)>& f, const Matrix& x0, double tol, int maxIter) {
    Matrix x = x0;
    if (x.rows() == 1 && x.cols() > 1) x = x.transpose();
    std::size_t n = x.size();
    if (n == 0) return x;

    std::vector<std::vector<double>> sim(n + 1, std::vector<double>(n));
    std::vector<double> fval(n + 1);

    for (std::size_t j = 0; j < n; ++j) sim[0][j] = x.rawData()[j];

    auto evalPoint = [&](const std::vector<double>& pt) -> double {
        Matrix m(x.rows(), x.cols());
        for (std::size_t j = 0; j < n; ++j) m.rawData()[j] = pt[j];
        return f((x0.rows() == 1) ? m.transpose() : m);
    };

    fval[0] = evalPoint(sim[0]);

    for (std::size_t i = 1; i <= n; ++i) {
        sim[i] = sim[0];
        if (std::abs(sim[0][i - 1]) > 1e-12) {
            sim[i][i - 1] += 0.05 * sim[0][i - 1];
        } else {
            sim[i][i - 1] += 0.00025;
        }
        fval[i] = evalPoint(sim[i]);
    }

    const double alpha = 1.0;
    const double gamma = 2.0;
    const double beta = 0.5;
    const double delta = 0.5;

    for (int iter = 0; iter < maxIter; ++iter) {
        std::vector<std::size_t> idx(n + 1);
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](std::size_t p, std::size_t q) {
            return fval[p] < fval[q];
        });

        std::vector<std::vector<double>> newSim(n + 1);
        std::vector<double> newFval(n + 1);
        for (std::size_t i = 0; i <= n; ++i) {
            newSim[i] = sim[idx[i]];
            newFval[i] = fval[idx[i]];
        }
        sim = std::move(newSim);
        fval = std::move(newFval);

        double maxDist = 0.0;
        for (std::size_t i = 1; i <= n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                maxDist = std::max(maxDist, std::abs(sim[i][j] - sim[0][j]));
            }
        }
        double fDiff = fval[n] - fval[0];
        if (maxDist < tol && std::abs(fDiff) < tol) {
            break;
        }

        std::vector<double> c(n, 0.0);
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                c[j] += sim[i][j];
            }
        }
        for (std::size_t j = 0; j < n; ++j) c[j] /= static_cast<double>(n);

        std::vector<double> xr(n);
        for (std::size_t j = 0; j < n; ++j) xr[j] = c[j] + alpha * (c[j] - sim[n][j]);
        double fr = evalPoint(xr);

        if (fr < fval[0]) {
            std::vector<double> xe(n);
            for (std::size_t j = 0; j < n; ++j) xe[j] = c[j] + gamma * (xr[j] - c[j]);
            double fe = evalPoint(xe);
            if (fe < fr) {
                sim[n] = xe;
                fval[n] = fe;
            } else {
                sim[n] = xr;
                fval[n] = fr;
            }
            continue;
        }

        if (fr < fval[n - 1]) {
            sim[n] = xr;
            fval[n] = fr;
            continue;
        }

        if (fr < fval[n]) {
            std::vector<double> xc(n);
            for (std::size_t j = 0; j < n; ++j) xc[j] = c[j] + beta * (xr[j] - c[j]);
            double fc = evalPoint(xc);
            if (fc <= fr) {
                sim[n] = xc;
                fval[n] = fc;
                continue;
            }
        } else {
            std::vector<double> xc(n);
            for (std::size_t j = 0; j < n; ++j) xc[j] = c[j] - beta * (c[j] - sim[n][j]);
            double fc = evalPoint(xc);
            if (fc < fval[n]) {
                sim[n] = xc;
                fval[n] = fc;
                continue;
            }
        }

        for (std::size_t i = 1; i <= n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                sim[i][j] = sim[0][j] + delta * (sim[i][j] - sim[0][j]);
            }
            fval[i] = evalPoint(sim[i]);
        }
    }

    Matrix res(x0.rows(), x0.cols());
    for (std::size_t j = 0; j < n; ++j) res.rawData()[j] = sim[0][j];
    return res;
}

Value fminsearch(const Value& funVal, const Value& x0Val, double tol, int maxIter) {
    if (!funVal.isFunctionHandle()) {
        throw std::runtime_error("fminsearch: first argument must be a function handle.");
    }
    bool isScalar = x0Val.isDouble();
    Matrix x0 = isScalar ? Matrix({{x0Val.toDouble()}}) : x0Val.toMatrix();

    auto f = [&funVal](const Matrix& x) -> double {
        Value res = funVal.call({Value(x)});
        if (res.isDouble()) return res.toDouble();
        if (res.isMatrix() && res.toMatrix().size() > 0) return res.toMatrix().rawData()[0];
        throw std::runtime_error("Objective function in fminsearch must return a numeric scalar.");
    };

    Matrix sol = fminsearch(f, x0, tol, maxIter);
    if (isScalar) return Value(sol(0, 0));
    return Value(sol);
}

// ----------------------------------------------------------------------------
// Ordinary Differential Equations (ode45, ode23)
// ----------------------------------------------------------------------------

OdeResult ode45(const std::function<Matrix(double, const Matrix&)>& f,
                double t0, double tf, const Matrix& y0, double rtol, double atol) {
    Matrix yCurrent = y0;
    if (yCurrent.rows() == 1 && yCurrent.cols() > 1) yCurrent = yCurrent.transpose();
    std::size_t m = yCurrent.size();

    std::vector<double> tRecord;
    std::vector<std::vector<double>> yRecord;

    double t = t0;
    tRecord.push_back(t);
    yRecord.push_back(yCurrent.rawData());

    const double c2 = 1.0 / 5.0, a21 = 1.0 / 5.0;
    const double c3 = 3.0 / 10.0, a31 = 3.0 / 40.0, a32 = 9.0 / 40.0;
    const double c4 = 4.0 / 5.0, a41 = 44.0 / 45.0, a42 = -56.0 / 15.0, a43 = 32.0 / 9.0;
    const double c5 = 8.0 / 9.0, a51 = 19372.0 / 6561.0, a52 = -25360.0 / 2187.0, a53 = 64448.0 / 6561.0, a54 = -212.0 / 729.0;
    const double c6 = 1.0, a61 = 9017.0 / 3168.0, a62 = -355.0 / 33.0, a63 = 46732.0 / 5247.0, a64 = 49.0 / 176.0, a65 = -5103.0 / 18656.0;
    const double b1 = 35.0 / 384.0, b3 = 500.0 / 1113.0, b4 = 125.0 / 192.0, b5 = -2187.0 / 6784.0, b6 = 11.0 / 84.0;
    const double e1 = 71.0 / 57600.0, e3 = -71.0 / 16695.0, e4 = 71.0 / 1920.0, e5 = -17253.0 / 339200.0, e6 = 22.0 / 525.0, e7 = -1.0 / 40.0;

    double dir = (tf >= t0) ? 1.0 : -1.0;
    double h = dir * std::min(std::abs(tf - t0) * 0.01, 0.1);

    auto addScaled = [m](const Matrix& base, double scale, const Matrix& k) -> Matrix {
        Matrix r(base.rows(), base.cols());
        for (std::size_t i = 0; i < m; ++i) {
            r.rawData()[i] = base.rawData()[i] + scale * k.rawData()[i];
        }
        return r;
    };

    int stepCount = 0;
    const int maxSteps = 100000;

    while (dir * (tf - t) > 1e-12 && stepCount < maxSteps) {
        ++stepCount;
        if (dir * (t + h - tf) > 0.0) {
            h = tf - t;
        }

        Matrix k1 = f(t, yCurrent);
        Matrix k2 = f(t + c2 * h, addScaled(yCurrent, h * a21, k1));
        
        Matrix y3 = yCurrent;
        for (std::size_t i = 0; i < m; ++i) y3.rawData()[i] += h * (a31 * k1.rawData()[i] + a32 * k2.rawData()[i]);
        Matrix k3 = f(t + c3 * h, y3);

        Matrix y4 = yCurrent;
        for (std::size_t i = 0; i < m; ++i) y4.rawData()[i] += h * (a41 * k1.rawData()[i] + a42 * k2.rawData()[i] + a43 * k3.rawData()[i]);
        Matrix k4 = f(t + c4 * h, y4);

        Matrix y5 = yCurrent;
        for (std::size_t i = 0; i < m; ++i) y5.rawData()[i] += h * (a51 * k1.rawData()[i] + a52 * k2.rawData()[i] + a53 * k3.rawData()[i] + a54 * k4.rawData()[i]);
        Matrix k5 = f(t + c5 * h, y5);

        Matrix y6 = yCurrent;
        for (std::size_t i = 0; i < m; ++i) y6.rawData()[i] += h * (a61 * k1.rawData()[i] + a62 * k2.rawData()[i] + a63 * k3.rawData()[i] + a64 * k4.rawData()[i] + a65 * k5.rawData()[i]);
        Matrix k6 = f(t + c6 * h, y6);

        Matrix yNext = yCurrent;
        for (std::size_t i = 0; i < m; ++i) {
            yNext.rawData()[i] += h * (b1 * k1.rawData()[i] + b3 * k3.rawData()[i] + b4 * k4.rawData()[i] + b5 * k5.rawData()[i] + b6 * k6.rawData()[i]);
        }

        Matrix k7 = f(t + h, yNext);

        // Error estimate
        double maxErrorRatio = 0.0;
        for (std::size_t i = 0; i < m; ++i) {
            double err = std::abs(h * (e1 * k1.rawData()[i] + e3 * k3.rawData()[i] + e4 * k4.rawData()[i] +
                                       e5 * k5.rawData()[i] + e6 * k6.rawData()[i] + e7 * k7.rawData()[i]));
            double tol = atol + rtol * std::max(std::abs(yCurrent.rawData()[i]), std::abs(yNext.rawData()[i]));
            maxErrorRatio = std::max(maxErrorRatio, err / tol);
        }

        if (maxErrorRatio <= 1.0 || std::abs(h) <= 1e-15) {
            t += h;
            yCurrent = yNext;
            tRecord.push_back(t);
            yRecord.push_back(yCurrent.rawData());

            double factor = (maxErrorRatio > 0.0) ? 0.9 * std::pow(1.0 / maxErrorRatio, 0.2) : 2.0;
            factor = std::min(5.0, std::max(0.2, factor));
            h *= factor;
        } else {
            double factor = 0.9 * std::pow(1.0 / maxErrorRatio, 0.25);
            factor = std::min(1.0, std::max(0.1, factor));
            h *= factor;
        }
    }

    std::size_t numPoints = tRecord.size();
    Matrix T(numPoints, 1);
    Matrix Y(numPoints, m);
    for (std::size_t i = 0; i < numPoints; ++i) {
        T(i, 0) = tRecord[i];
        for (std::size_t j = 0; j < m; ++j) {
            Y(i, j) = yRecord[i][j];
        }
    }

    return {T, Y};
}

OdeResult ode45(const Value& funVal, const Value& tspanVal, const Value& y0Val, double rtol, double atol) {
    if (!funVal.isFunctionHandle()) {
        throw std::runtime_error("ode45: first argument must be a function handle.");
    }
    double t0 = 0.0;
    double tf = 1.0;
    if (tspanVal.isMatrix()) {
        const auto& m = tspanVal.toMatrix();
        if (m.size() < 2) throw std::runtime_error("ode45: tspan must contain at least [t0, tf].");
        t0 = m.rawData()[0];
        tf = m.rawData()[m.size() - 1];
    } else {
        throw std::runtime_error("ode45: tspan must be a numeric vector [t0, tf].");
    }

    Matrix y0 = y0Val.isDouble() ? Matrix({{y0Val.toDouble()}}) : y0Val.toMatrix();

    auto f = [&funVal](double t, const Matrix& y) -> Matrix {
        Value res = funVal.call({Value(t), Value(y)});
        if (res.isDouble()) return Matrix({{res.toDouble()}});
        if (res.isMatrix()) {
            Matrix m = res.toMatrix();
            if (m.rows() == 1 && m.cols() > 1) return m.transpose();
            return m;
        }
        throw std::runtime_error("ODE function must return a numeric derivative.");
    };

    return ode45(f, t0, tf, y0, rtol, atol);
}

OdeResult ode23(const std::function<Matrix(double, const Matrix&)>& f,
                double t0, double tf, const Matrix& y0, double rtol, double atol) {
    Matrix yCurrent = y0;
    if (yCurrent.rows() == 1 && yCurrent.cols() > 1) yCurrent = yCurrent.transpose();
    std::size_t m = yCurrent.size();

    std::vector<double> tRecord;
    std::vector<std::vector<double>> yRecord;

    double t = t0;
    tRecord.push_back(t);
    yRecord.push_back(yCurrent.rawData());

    const double c2 = 1.0 / 2.0, a21 = 1.0 / 2.0;
    const double c3 = 3.0 / 4.0, a32 = 3.0 / 4.0;
    const double b1 = 2.0 / 9.0, b2 = 1.0 / 3.0, b3 = 4.0 / 9.0;
    const double e1 = -5.0 / 72.0, e2 = 1.0 / 12.0, e3 = 1.0 / 9.0, e4 = -1.0 / 8.0;

    double dir = (tf >= t0) ? 1.0 : -1.0;
    double h = dir * std::min(std::abs(tf - t0) * 0.01, 0.1);

    int stepCount = 0;
    const int maxSteps = 100000;

    while (dir * (tf - t) > 1e-12 && stepCount < maxSteps) {
        ++stepCount;
        if (dir * (t + h - tf) > 0.0) {
            h = tf - t;
        }

        Matrix k1 = f(t, yCurrent);
        
        Matrix y2 = yCurrent;
        for (std::size_t i = 0; i < m; ++i) y2.rawData()[i] += (h * a21) * k1.rawData()[i];
        Matrix k2 = f(t + c2 * h, y2);

        Matrix y3 = yCurrent;
        for (std::size_t i = 0; i < m; ++i) y3.rawData()[i] += (h * a32) * k2.rawData()[i];
        Matrix k3 = f(t + c3 * h, y3);

        Matrix yNext = yCurrent;
        for (std::size_t i = 0; i < m; ++i) {
            yNext.rawData()[i] += h * (b1 * k1.rawData()[i] + b2 * k2.rawData()[i] + b3 * k3.rawData()[i]);
        }

        Matrix k4 = f(t + h, yNext);

        double maxErrorRatio = 0.0;
        for (std::size_t i = 0; i < m; ++i) {
            double err = std::abs(h * (e1 * k1.rawData()[i] + e2 * k2.rawData()[i] + e3 * k3.rawData()[i] + e4 * k4.rawData()[i]));
            double tol = atol + rtol * std::max(std::abs(yCurrent.rawData()[i]), std::abs(yNext.rawData()[i]));
            maxErrorRatio = std::max(maxErrorRatio, err / tol);
        }

        if (maxErrorRatio <= 1.0 || std::abs(h) <= 1e-15) {
            t += h;
            yCurrent = yNext;
            tRecord.push_back(t);
            yRecord.push_back(yCurrent.rawData());

            double factor = (maxErrorRatio > 0.0) ? 0.9 * std::pow(1.0 / maxErrorRatio, 1.0 / 3.0) : 2.0;
            factor = std::min(5.0, std::max(0.2, factor));
            h *= factor;
        } else {
            double factor = 0.9 * std::pow(1.0 / maxErrorRatio, 1.0 / 3.0);
            factor = std::min(1.0, std::max(0.1, factor));
            h *= factor;
        }
    }

    std::size_t numPoints = tRecord.size();
    Matrix T(numPoints, 1);
    Matrix Y(numPoints, m);
    for (std::size_t i = 0; i < numPoints; ++i) {
        T(i, 0) = tRecord[i];
        for (std::size_t j = 0; j < m; ++j) {
            Y(i, j) = yRecord[i][j];
        }
    }

    return {T, Y};
}

OdeResult ode23(const Value& funVal, const Value& tspanVal, const Value& y0Val, double rtol, double atol) {
    if (!funVal.isFunctionHandle()) {
        throw std::runtime_error("ode23: first argument must be a function handle.");
    }
    double t0 = 0.0;
    double tf = 1.0;
    if (tspanVal.isMatrix()) {
        const auto& m = tspanVal.toMatrix();
        if (m.size() < 2) throw std::runtime_error("ode23: tspan must contain at least [t0, tf].");
        t0 = m.rawData()[0];
        tf = m.rawData()[m.size() - 1];
    } else {
        throw std::runtime_error("ode23: tspan must be a numeric vector [t0, tf].");
    }

    Matrix y0 = y0Val.isDouble() ? Matrix({{y0Val.toDouble()}}) : y0Val.toMatrix();

    auto f = [&funVal](double t, const Matrix& y) -> Matrix {
        Value res = funVal.call({Value(t), Value(y)});
        if (res.isDouble()) return Matrix({{res.toDouble()}});
        if (res.isMatrix()) {
            Matrix m = res.toMatrix();
            if (m.rows() == 1 && m.cols() > 1) return m.transpose();
            return m;
        }
        throw std::runtime_error("ODE function must return a numeric derivative.");
    };

    return ode23(f, t0, tf, y0, rtol, atol);
}

} // namespace matforge::core::math

#pragma once

#include "matforge/core/Matrix.hpp"
#include "matforge/core/Value.hpp"

#include <functional>
#include <vector>

namespace matforge::core::math {

// Elementary mathematical functions
Value sin(const Value& val);
Value cos(const Value& val);
Value tan(const Value& val);
Value sqrt(const Value& val);
Value exp(const Value& val);
Value log(const Value& val);
Value abs(const Value& val);

Matrix apply(const Matrix& m, double (*func)(double));

// Differences & Numerical Gradient
Matrix diff(const Matrix& x, int n = 1);
Matrix gradient(const Matrix& y, double h = 1.0);

// Numerical Quadrature (Trapezoidal Rule)
double trapz(const Matrix& y);
double trapz(const Matrix& x, const Matrix& y);
Matrix cumtrapz(const Matrix& y);
Matrix cumtrapz(const Matrix& x, const Matrix& y);

// Adaptive Simpson's Quadrature (integral)
double integral(const std::function<double(double)>& f, double a, double b, double tol = 1e-10);
double integral(const Value& funVal, double a, double b, double tol = 1e-10);

// Root Finding: Brent's Method (fzero)
double fzero(const std::function<double(double)>& f, double x0, double tol = 1e-12);
double fzero(const Value& funVal, double x0, double tol = 1e-12);

// Nonlinear Equation Solver (fsolve)
Matrix fsolve(const std::function<Matrix(const Matrix&)>& F, const Matrix& x0, double tol = 1e-8, int maxIter = 200);
Value fsolve(const Value& funVal, const Value& x0Val, double tol = 1e-8, int maxIter = 200);

// Linear Equation Solver: Ax = b
Value linsolve(const Value& A, const Value& b);

// Unified Solve: dispatches to linsolve if A is numeric, fsolve if A is a function handle
Value solve(const Value& a, const Value& b);

// Multivariable Optimization: Nelder-Mead Simplex (fminsearch)
Matrix fminsearch(const std::function<double(const Matrix&)>& f, const Matrix& x0, double tol = 1e-6, int maxIter = 2000);
Value fminsearch(const Value& funVal, const Value& x0Val, double tol = 1e-6, int maxIter = 2000);

// Ordinary Differential Equations (ODE) Solvers
struct OdeResult {
    Matrix t; // N x 1 column vector
    Matrix y; // N x M matrix (each row is state at t[i])
};

OdeResult ode45(const std::function<Matrix(double, const Matrix&)>& f, double t0, double tf, const Matrix& y0, double rtol = 1e-4, double atol = 1e-6);
OdeResult ode45(const Value& funVal, const Value& tspanVal, const Value& y0Val, double rtol = 1e-4, double atol = 1e-6);

OdeResult ode23(const std::function<Matrix(double, const Matrix&)>& f, double t0, double tf, const Matrix& y0, double rtol = 1e-3, double atol = 1e-5);
OdeResult ode23(const Value& funVal, const Value& tspanVal, const Value& y0Val, double rtol = 1e-3, double atol = 1e-5);

} // namespace matforge::core::math

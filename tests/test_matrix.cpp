#include "matforge/core/Matrix.hpp"
#include "matforge/core/MathUtils.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

void testConstructorsAndIdentity() {
    using namespace matforge::core;

    Matrix m1(2, 3, 5.0);
    assert(m1.rows() == 2);
    assert(m1.cols() == 3);
    assert(m1(0, 0) == 5.0);
    assert(m1(1, 2) == 5.0);

    Matrix eyeMat = Matrix::eye(3);
    assert(eyeMat.rows() == 3 && eyeMat.cols() == 3);
    assert(eyeMat(0, 0) == 1.0 && eyeMat(1, 1) == 1.0 && eyeMat(2, 2) == 1.0);
    assert(eyeMat(0, 1) == 0.0 && eyeMat(1, 0) == 0.0);

    Matrix lin = Matrix::linspace(0.0, 10.0, 5);
    assert(lin.cols() == 5);
    assert(lin(0, 0) == 0.0);
    assert(lin(0, 4) == 10.0);
    assert(lin(0, 2) == 5.0);
    std::cout << "[PASS] Constructors and generators\n";
}

void testArithmetic() {
    using namespace matforge::core;

    Matrix A = {{1, 2}, {3, 4}};
    Matrix B = {{2, 0}, {1, 2}};

    // Addition
    Matrix C = A + B;
    assert(C(0, 0) == 3.0 && C(0, 1) == 2.0);
    assert(C(1, 0) == 4.0 && C(1, 1) == 6.0);

    // Multiplication: [[1, 2], [3, 4]] * [[2, 0], [1, 2]]
    // (0,0) = 1*2 + 2*1 = 4
    // (0,1) = 1*0 + 2*2 = 4
    // (1,0) = 3*2 + 4*1 = 10
    // (1,1) = 3*0 + 4*2 = 8
    Matrix D = A * B;
    assert(D(0, 0) == 4.0 && D(0, 1) == 4.0);
    assert(D(1, 0) == 10.0 && D(1, 1) == 8.0);

    // Transpose
    Matrix At = A.transpose();
    assert(At(0, 1) == 3.0 && At(1, 0) == 2.0);

    // Determinant
    // det(A) = 1*4 - 2*3 = -2
    assert(std::abs(A.determinant() - (-2.0)) < 1e-9);

    std::cout << "[PASS] Matrix arithmetic and linear algebra\n";
}

void testPhase2LinearAlgebra() {
    using namespace matforge::core;

    Matrix A = {{4, 7}, {2, 6}};
    Matrix invA = A.inverse();
    Matrix I = A * invA;
    assert(std::abs(I(0, 0) - 1.0) < 1e-9);
    assert(std::abs(I(1, 1) - 1.0) < 1e-9);
    assert(std::abs(I(0, 1)) < 1e-9);
    assert(std::abs(I(1, 0)) < 1e-9);

    // Solve A \ b
    Matrix b = {{1}, {2}};
    Matrix x = A.solve(b);
    Matrix bRecon = A * x;
    assert(std::abs(bRecon(0, 0) - b(0, 0)) < 1e-9);
    assert(std::abs(bRecon(1, 0) - b(1, 0)) < 1e-9);

    // Right solve B / A
    Matrix B = {{3, 5}};
    Matrix X = B.rightSolve(A);
    Matrix BRecon = X * A;
    assert(std::abs(BRecon(0, 0) - B(0, 0)) < 1e-9);
    assert(std::abs(BRecon(0, 1) - B(0, 1)) < 1e-9);

    // Rank & Condition
    assert(A.rank() == 2);
    assert(A.cond() > 1.0);

    // Pseudo-inverse: A * pinv(A) * A == A
    Matrix pinvA = A.pinv();
    Matrix checkPinv = A * pinvA * A;
    assert(std::abs(checkPinv(0, 0) - A(0, 0)) < 1e-8);
    assert(std::abs(checkPinv(1, 1) - A(1, 1)) < 1e-8);

    // QR decomposition: Q * R == A, Q' * Q == I
    auto [Q, R] = A.qr();
    Matrix qrCheck = Q * R;
    assert(std::abs(qrCheck(0, 0) - A(0, 0)) < 1e-8);
    assert(std::abs(qrCheck(1, 1) - A(1, 1)) < 1e-8);
    Matrix qOrth = Q.transpose() * Q;
    assert(std::abs(qOrth(0, 0) - 1.0) < 1e-8);
    assert(std::abs(qOrth(1, 1) - 1.0) < 1e-8);

    // Cholesky decomposition of symmetric positive-definite matrix
    Matrix S = {{4, 2}, {2, 5}};
    Matrix R_chol = S.chol();
    Matrix cholCheck = R_chol.transpose() * R_chol;
    assert(std::abs(cholCheck(0, 0) - S(0, 0)) < 1e-8);
    assert(std::abs(cholCheck(1, 1) - S(1, 1)) < 1e-8);

    // SVD: U * S * V' == A
    auto [U, Smat, V] = A.svd();
    Matrix svdCheck = U * Smat * V.transpose();
    assert(std::abs(svdCheck(0, 0) - A(0, 0)) < 1e-8);
    assert(std::abs(svdCheck(1, 1) - A(1, 1)) < 1e-8);

    std::cout << "[PASS] Advanced Linear Algebra (inv, solve, pinv, rank, cond, QR, Chol, SVD)\n";
}

void testPhase2StatsAndRandom() {
    using namespace matforge::core;

    Matrix::setSeed(42);
    Matrix r = Matrix::rand(5, 5);
    assert(r.rows() == 5 && r.cols() == 5);
    for (double v : r.rawData()) {
        assert(v >= 0.0 && v < 1.0);
    }

    Matrix ri = Matrix::randi(10, 4, 4);
    assert(ri.rows() == 4 && ri.cols() == 4);
    for (double v : ri.rawData()) {
        assert(v >= 1.0 && v <= 10.0);
    }

    Matrix sample = {{1, 2, 3, 4, 5}};
    assert(sample.mean() == 3.0);
    assert(sample.variance() == 2.5);
    assert(std::abs(sample.stdDev() - std::sqrt(2.5)) < 1e-9);
    assert(sample.median() == 3.0);

    Matrix evenSample = {{1, 2, 8, 10}};
    assert(evenSample.median() == 5.0); // (2 + 8) / 2

    std::cout << "[PASS] Statistics and Random Numbers\n";
}

void testPhase2SignalAndIO() {
    using namespace matforge::core;

    // Convolution: [1, 2, 3] * [4, 5] = [4, 13, 22, 15]
    Matrix u = {{1, 2, 3}};
    Matrix v = {{4, 5}};
    Matrix w = Matrix::conv(u, v);
    assert(w.size() == 4);
    assert(w(0, 0) == 4.0);
    assert(w(0, 1) == 13.0);
    assert(w(0, 2) == 22.0);
    assert(w(0, 3) == 15.0);

    // FFT & IFFT round-trip
    Matrix sig = {{1.0, 2.0, 3.0, 4.0}};
    Matrix Y = sig.fft();
    assert(Y.rows() == 4 && Y.cols() == 2);
    Matrix reconstructed = Y.ifft();
    assert(reconstructed.size() == 4);
    for (std::size_t i = 0; i < 4; ++i) {
        assert(std::abs(reconstructed.rawData()[i] - sig.rawData()[i]) < 1e-8);
    }

    // CSV write and read
    Matrix original = {{1.5, 2.5, 3.5}, {4.5, 5.5, 6.5}};
    std::string testPath = "temp_test_io.csv";
    original.writeCSV(testPath);

    Matrix readBack = Matrix::readCSV(testPath);
    assert(readBack.rows() == 2 && readBack.cols() == 3);
    for (std::size_t r = 0; r < 2; ++r) {
        for (std::size_t c = 0; c < 3; ++c) {
            assert(std::abs(readBack(r, c) - original(r, c)) < 1e-8);
        }
    }
    std::remove(testPath.c_str());

    std::cout << "[PASS] Signal processing (conv, FFT, IFFT) and CSV I/O\n";
}

void testPhase4MathUtils() {
    using namespace matforge::core;

    // 1. diff & gradient
    Matrix v = {{1.0, 4.0, 9.0, 16.0}};
    Matrix d = math::diff(v);
    assert(d.size() == 3);
    assert(d(0, 0) == 3.0 && d(0, 1) == 5.0 && d(0, 2) == 7.0);

    Matrix g = math::gradient(v, 1.0);
    assert(g.size() == 4);
    assert(g(0, 0) == 3.0 && g(0, 1) == 4.0 && g(0, 2) == 6.0 && g(0, 3) == 7.0);

    // 2. trapz & cumtrapz
    Matrix x = {{0.0, 1.0, 2.0, 3.0}};
    Matrix y = {{0.0, 1.0, 4.0, 9.0}};
    double tz = math::trapz(x, y);
    assert(std::abs(tz - 9.5) < 1e-9);

    Matrix ctz = math::cumtrapz(x, y);
    assert(ctz.size() == 4 && std::abs(ctz(0, 3) - 9.5) < 1e-9);

    // 3. Adaptive Simpson Quadrature
    double q = math::integral([](double t) { return t * t; }, 0.0, 1.0);
    assert(std::abs(q - 1.0 / 3.0) < 1e-8);

    // 4. Brent Root Finding
    double root = math::fzero([](double t) { return t * t - 4.0; }, 3.0);
    assert(std::abs(root - 2.0) < 1e-8);

    // 5. Newton-Raphson nonlinear solver (fsolve)
    Matrix x0 = {{2.0}};
    Matrix sol = math::fsolve([](const Matrix& m) { return Matrix({{m(0, 0) * m(0, 0) * m(0, 0) - 27.0}}); }, x0);
    assert(std::abs(sol(0, 0) - 3.0) < 1e-6);

    // 6. Nelder-Mead Optimization (fminsearch)
    Matrix opt = math::fminsearch([](const Matrix& m) { return (m(0, 0) - 5.0) * (m(0, 0) - 5.0) + 2.0; }, x0);
    assert(std::abs(opt(0, 0) - 5.0) < 1e-5);

    // 7. ODE Solvers (ode45 & ode23)
    Matrix y0 = {{1.0}};
    auto res45 = math::ode45([](double /*t*/, const Matrix& yVal) { return Matrix({{-2.0 * yVal(0, 0)}}); }, 0.0, 1.0, y0);
    assert(res45.t.rows() > 5);
    double finalY45 = res45.y(res45.y.rows() - 1, 0);
    assert(std::abs(finalY45 - std::exp(-2.0)) < 1e-3);

    auto res23 = math::ode23([](double /*t*/, const Matrix& yVal) { return Matrix({{-2.0 * yVal(0, 0)}}); }, 0.0, 1.0, y0);
    assert(res23.t.rows() > 5);
    double finalY23 = res23.y(res23.y.rows() - 1, 0);
    assert(std::abs(finalY23 - std::exp(-2.0)) < 1e-2);

    std::cout << "[PASS] MathUtils (diff, gradient, trapz, cumtrapz, integral, fzero, fsolve, fminsearch, ode45, ode23)\n";
}

int main() {
    std::cout << "Running Matrix unit tests...\n";
    testConstructorsAndIdentity();
    testArithmetic();
    testPhase2LinearAlgebra();
    testPhase2StatsAndRandom();
    testPhase2SignalAndIO();
    testPhase4MathUtils();
    std::cout << "All Matrix unit tests passed!\n";
    return 0;
}


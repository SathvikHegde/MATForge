#include "matforge/core/Matrix.hpp"
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

int main() {
    std::cout << "Running Matrix unit tests...\n";
    testConstructorsAndIdentity();
    testArithmetic();
    testPhase2LinearAlgebra();
    testPhase2StatsAndRandom();
    testPhase2SignalAndIO();
    std::cout << "All Matrix unit tests passed!\n";
    return 0;
}


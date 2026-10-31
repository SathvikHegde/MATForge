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

int main() {
    std::cout << "Running Matrix unit tests...\n";
    testConstructorsAndIdentity();
    testArithmetic();
    std::cout << "All Matrix unit tests passed!\n";
    return 0;
}


#include "matforge/engine/Interpreter.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

void testAssignmentsAndMath() {
    matforge::engine::Interpreter interpreter;

    // Scalar assignment
    auto res1 = interpreter.execute("x = 42;");
    assert(res1.success);
    assert(res1.output.empty()); // Suppressed with semicolon
    assert(interpreter.workspace()->get("x").toDouble() == 42.0);

    // Expression evaluation
    auto res2 = interpreter.execute("y = x + 8;");
    assert(res2.success);
    assert(interpreter.workspace()->get("y").toDouble() == 50.0);

    // Built-in function
    auto res3 = interpreter.execute("s = sqrt(y * 2);");
    assert(res3.success);
    assert(interpreter.workspace()->get("s").toDouble() == 10.0);

    std::cout << "[PASS] Basic assignments and math\n";
}

void testMatrixEvaluation() {
    matforge::engine::Interpreter interpreter;

    // Matrix literal and assignment
    auto res1 = interpreter.execute("A = [1, 2; 3, 4];");
    assert(res1.success);
    auto matA = interpreter.workspace()->get("A").toMatrix();
    assert(matA.rows() == 2 && matA.cols() == 2);
    assert(matA(0, 0) == 1.0 && matA(1, 1) == 4.0);

    // Matrix multiplication
    auto res2 = interpreter.execute("b = [2; 3]; x = A * b;");
    assert(res2.success);
    auto matX = interpreter.workspace()->get("x").toMatrix();
    // [1*2 + 2*3; 3*2 + 4*3] = [8; 18]
    assert(matX.rows() == 2 && matX.cols() == 1);
    assert(matX(0, 0) == 8.0 && matX(1, 0) == 18.0);

    // Matrix indexing (1-based MATLAB convention)
    auto res3 = interpreter.execute("val = A(2, 1);");
    assert(res3.success);
    assert(interpreter.workspace()->get("val").toDouble() == 3.0);

    std::cout << "[PASS] Matrix evaluation and indexing\n";
}

int main() {
    std::cout << "Running Interpreter unit tests...\n";
    testAssignmentsAndMath();
    testMatrixEvaluation();
    std::cout << "All Interpreter unit tests passed!\n";
    return 0;
}


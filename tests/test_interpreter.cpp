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
    assert(matX.rows() == 2 && matX.cols() == 1);
    assert(matX(0, 0) == 8.0 && matX(1, 0) == 18.0);

    // Matrix indexing (1-based MATLAB convention)
    auto res3 = interpreter.execute("val = A(2, 1);");
    assert(res3.success);
    assert(interpreter.workspace()->get("val").toDouble() == 3.0);

    std::cout << "[PASS] Matrix evaluation and indexing\n";
}

void testColonAndSlicing() {
    matforge::engine::Interpreter interpreter;

    // Colon ranges
    auto res1 = interpreter.execute("v = 1:5;");
    assert(res1.success);
    auto v = interpreter.workspace()->get("v").toMatrix();
    assert(v.cols() == 5);
    assert(v(0, 0) == 1.0 && v(0, 4) == 5.0);

    auto res2 = interpreter.execute("stepRange = 0:0.5:2;");
    assert(res2.success);
    auto sr = interpreter.workspace()->get("stepRange").toMatrix();
    assert(sr.cols() == 5);
    assert(sr(0, 1) == 0.5 && sr(0, 4) == 2.0);

    // 2D Slicing: A(1:2, :) and end keyword
    auto res3 = interpreter.execute("M = [1, 2, 3; 4, 5, 6; 7, 8, 9];");
    assert(res3.success);

    auto res4 = interpreter.execute("sub = M(1:2, 2:3);");
    assert(res4.success);
    auto sub = interpreter.workspace()->get("sub").toMatrix();
    assert(sub.rows() == 2 && sub.cols() == 2);
    assert(sub(0, 0) == 2.0 && sub(0, 1) == 3.0);
    assert(sub(1, 0) == 5.0 && sub(1, 1) == 6.0);

    // Slicing with end
    auto res5 = interpreter.execute("lastCol = M(:, end);");
    assert(res5.success);
    auto lc = interpreter.workspace()->get("lastCol").toMatrix();
    assert(lc.rows() == 3 && lc.cols() == 1);
    assert(lc(0, 0) == 3.0 && lc(1, 0) == 6.0 && lc(2, 0) == 9.0);

    auto res6 = interpreter.execute("corner = M(2:end, end-1:end);");
    assert(res6.success);
    auto cr = interpreter.workspace()->get("corner").toMatrix();
    assert(cr.rows() == 2 && cr.cols() == 2);
    assert(cr(0, 0) == 5.0 && cr(1, 1) == 9.0);

    std::cout << "[PASS] Colon ranges and matrix slicing\n";
}

void testSubscriptAssignment() {
    matforge::engine::Interpreter interpreter;

    // Single element assignment
    auto res1 = interpreter.execute("A = [1, 2; 3, 4]; A(1, 2) = 99;");
    assert(res1.success);
    auto matA = interpreter.workspace()->get("A").toMatrix();
    assert(matA(0, 1) == 99.0);

    // Row replacement with scalar
    auto res2 = interpreter.execute("A(2, :) = 0;");
    assert(res2.success);
    matA = interpreter.workspace()->get("A").toMatrix();
    assert(matA(1, 0) == 0.0 && matA(1, 1) == 0.0);

    // Matrix auto-expansion
    auto res3 = interpreter.execute("A(4, 4) = 42;");
    assert(res3.success);
    matA = interpreter.workspace()->get("A").toMatrix();
    assert(matA.rows() == 4 && matA.cols() == 4);
    assert(matA(3, 3) == 42.0);
    assert(matA(2, 2) == 0.0); // Filled with 0

    std::cout << "[PASS] Subscript assignment and auto-expansion\n";
}

void testControlFlow() {
    matforge::engine::Interpreter interpreter;

    // For loop
    auto res1 = interpreter.execute(
        "total = 0;\n"
        "for i = 1:5\n"
        "    total = total + i;\n"
        "end\n"
    );
    assert(res1.success);
    assert(interpreter.workspace()->get("total").toDouble() == 15.0);

    // While loop
    auto res2 = interpreter.execute(
        "count = 1;\n"
        "while count < 10\n"
        "    count = count * 2;\n"
        "end\n"
    );
    assert(res2.success);
    assert(interpreter.workspace()->get("count").toDouble() == 16.0);

    // If - elseif - else
    auto res3 = interpreter.execute(
        "val = 7;\n"
        "out = 0;\n"
        "if val > 10\n"
        "    out = 1;\n"
        "elseif val > 5\n"
        "    out = 2;\n"
        "else\n"
        "    out = 3;\n"
        "end\n"
    );
    assert(res3.success);
    assert(interpreter.workspace()->get("out").toDouble() == 2.0);

    // Break statement
    auto res4 = interpreter.execute(
        "sumBreak = 0;\n"
        "for i = 1:100\n"
        "    if i > 5\n"
        "        break;\n"
        "    end\n"
        "    sumBreak = sumBreak + i;\n"
        "end\n"
    );
    assert(res4.success);
    assert(interpreter.workspace()->get("sumBreak").toDouble() == 15.0);

    std::cout << "[PASS] Control flow (for, while, if/elseif/else, break)\n";
}

void testUserFunctions() {
    matforge::engine::Interpreter interpreter;

    // Define function and call
    auto res1 = interpreter.execute(
        "function y = square(x)\n"
        "    y = x * x;\n"
        "end\n"
        "ans1 = square(6);\n"
    );
    assert(res1.success);
    assert(interpreter.workspace()->get("ans1").toDouble() == 36.0);

    // Function with local scope (should not leak local vars to outer workspace)
    auto res2 = interpreter.execute(
        "function res = compute(a, b)\n"
        "    temp = a + b;\n"
        "    res = temp * 2;\n"
        "end\n"
        "ans2 = compute(3, 4);\n"
    );
    assert(res2.success);
    assert(interpreter.workspace()->get("ans2").toDouble() == 14.0);
    assert(!interpreter.workspace()->has("temp")); // 'temp' remained local!

    std::cout << "[PASS] User-defined functions\n";
}

int main() {
    std::cout << "Running Interpreter unit tests...\n";
    testAssignmentsAndMath();
    testMatrixEvaluation();
    testColonAndSlicing();
    testSubscriptAssignment();
    testControlFlow();
    testUserFunctions();
    std::cout << "All Interpreter unit tests passed!\n";
    return 0;
}

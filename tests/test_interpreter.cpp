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

void testPhase2Interpreter() {
    matforge::engine::Interpreter interpreter;

    // Linear solve with backslash: A \ b
    auto res1 = interpreter.execute(
        "A = [4, 7; 2, 6];\n"
        "b = [1; 2];\n"
        "x = A \\ b;\n"
        "diff = A * x - b;\n"
    );
    assert(res1.success);
    auto diff = interpreter.workspace()->get("diff").toMatrix();
    assert(std::abs(diff(0, 0)) < 1e-8 && std::abs(diff(1, 0)) < 1e-8);

    // Right solve: B / A
    auto res2 = interpreter.execute(
        "B = [3, 5];\n"
        "X = B / A;\n"
        "reconB = X * A;\n"
    );
    assert(res2.success);
    auto reconB = interpreter.workspace()->get("reconB").toMatrix();
    assert(std::abs(reconB(0, 0) - 3.0) < 1e-8);
    assert(std::abs(reconB(0, 1) - 5.0) < 1e-8);

    // Dot-backslash: A .\ B
    auto res3 = interpreter.execute(
        "D1 = [2, 4]; D2 = [6, 12];\n"
        "div = D1 .\\ D2;\n"
    );
    assert(res3.success);
    auto div = interpreter.workspace()->get("div").toMatrix();
    assert(div(0, 0) == 3.0 && div(0, 1) == 3.0);

    // Built-in linear algebra functions
    auto res4 = interpreter.execute(
        "invA = inv(A);\n"
        "pA = pinv(A);\n"
        "r = rank(A);\n"
        "c = cond(A);\n"
        "R_chol = chol([4, 2; 2, 5]);\n"
    );
    assert(res4.success);
    assert(interpreter.workspace()->get("r").toDouble() == 2.0);
    assert(interpreter.workspace()->get("c").toDouble() > 1.0);

    // Multi-Variable Assignment: [U, S, V] = svd(A)
    auto res5 = interpreter.execute(
        "[U, S, V] = svd(A);\n"
        "[Q, R] = qr(A);\n"
        "[L, U_lu] = lu(A);\n"
    );
    assert(res5.success);
    assert(interpreter.workspace()->has("U"));
    assert(interpreter.workspace()->has("S"));
    assert(interpreter.workspace()->has("V"));
    assert(interpreter.workspace()->has("Q"));
    assert(interpreter.workspace()->has("R"));
    assert(interpreter.workspace()->has("L"));
    assert(interpreter.workspace()->has("U_lu"));

    // Random & Statistics
    auto res6 = interpreter.execute(
        "rng(42);\n"
        "Rnd = rand(3, 3);\n"
        "stdev = std(1:5);\n"
        "med = median([5, 1, 9, 3, 7]);\n"
    );
    assert(res6.success);
    assert(std::abs(interpreter.workspace()->get("stdev").toDouble() - std::sqrt(2.5)) < 1e-8);
    assert(interpreter.workspace()->get("med").toDouble() == 5.0);

    // Signal processing: conv, fft, ifft
    auto res7 = interpreter.execute(
        "sig = [1, 2, 3, 4];\n"
        "Y = fft(sig);\n"
        "recon = ifft(Y);\n"
        "c = conv([1, 2], [3, 4]);\n"
    );
    assert(res7.success);
    auto recon = interpreter.workspace()->get("recon").toMatrix();
    assert(std::abs(recon(0, 0) - 1.0) < 1e-8);
    assert(std::abs(recon(3, 0) - 4.0) < 1e-8);
    auto c = interpreter.workspace()->get("c").toMatrix();
    assert(c(0, 0) == 3.0 && c(0, 1) == 10.0 && c(0, 2) == 8.0);

    // File I/O: writematrix and readmatrix
    auto res8 = interpreter.execute(
        "writematrix([11, 22; 33, 44], 'test_interp_io.csv');\n"
        "M_read = readmatrix('test_interp_io.csv');\n"
    );
    assert(res8.success);
    auto M_read = interpreter.workspace()->get("M_read").toMatrix();
    assert(M_read(0, 0) == 11.0 && M_read(1, 1) == 44.0);
    std::remove("test_interp_io.csv");

    std::cout << "[PASS] Phase 2 Interpreter features (\\, mrdivide, .\\, multi-assign, built-ins, I/O)\n";
}

void testPhase3GraphicsCommands() {
    matforge::engine::Interpreter interpreter;

    bool plotCalled = false;
    std::string plotFmt;
    std::string plotType;
    interpreter.setPlotCallback([&](const matforge::engine::Interpreter::PlotCommand& cmd) {
        plotCalled = true;
        plotFmt = cmd.format;
        plotType = cmd.plotType;
    });

    bool holdState = false;
    interpreter.setHoldCallback([&](bool h) { holdState = h; });

    bool gridState = false;
    interpreter.setGridCallback([&](bool g) { gridState = g; });

    std::string plotTitle;
    interpreter.setTitleCallback([&](const std::string& t) { plotTitle = t; });

    std::string xLabel, yLabel;
    interpreter.setLabelCallback([&](const std::string& l, bool isX) {
        if (isX) xLabel = l; else yLabel = l;
    });

    std::vector<std::string> legends;
    interpreter.setLegendCallback([&](const std::vector<std::string>& l) { legends = l; });

    int subR = 0, subC = 0, subP = 0;
    interpreter.setSubplotCallback([&](int r, int c, int p) {
        subR = r; subC = c; subP = p;
    });

    // Test formatted plot
    auto res1 = interpreter.execute("x = 0:0.1:1; y = x.^2; plot(x, y, 'r--*');");
    assert(res1.success);
    assert(plotCalled);
    assert(plotFmt == "r--*");
    assert(plotType == "plot");

    // Test bar and stem
    auto res2 = interpreter.execute("bar([1, 2, 3]);");
    assert(res2.success);
    assert(plotType == "bar");

    auto res3 = interpreter.execute("stem(x, y);");
    assert(res3.success);
    assert(plotType == "stem");

    // Test hold on & hold off commands
    auto res4 = interpreter.execute("hold on;");
    assert(res4.success);
    assert(holdState == true);

    auto res5 = interpreter.execute("hold off;");
    assert(res5.success);
    assert(holdState == false);

    // Test grid on & grid off
    auto res6 = interpreter.execute("grid on;");
    assert(res6.success);
    assert(gridState == true);

    // Test title, xlabel, ylabel, legend
    auto res7 = interpreter.execute(
        "title('Parabola');\n"
        "xlabel('Time (s)');\n"
        "ylabel('Velocity');\n"
        "legend('First', 'Second');\n"
    );
    assert(res7.success);
    assert(plotTitle == "Parabola");
    assert(xLabel == "Time (s)");
    assert(yLabel == "Velocity");
    assert(legends.size() == 2 && legends[0] == "First" && legends[1] == "Second");

    // Test subplot
    auto res8 = interpreter.execute("subplot(2, 2, 3);");
    assert(res8.success);
    assert(subR == 2 && subC == 2 && subP == 3);

    std::cout << "[PASS] Phase 3 Graphics and Plotting commands (plot, bar, stem, hold, grid, title, labels, subplot)\n";
}

void testPhase4() {
    matforge::engine::Interpreter interp;

    // 1. Anonymous functions and invocation
    auto res1 = interp.execute(
        "f = @(x) x.^2 - 4;\n"
        "val = f(3);\n"
        "g = @(x, y) x + 2*y;\n"
        "val2 = g(3, 4);\n"
        "h = @sin;\n"
        "val3 = h(0);\n"
    );
    assert(res1.success);
    assert(interp.workspace()->get("val").toDouble() == 5.0);
    assert(interp.workspace()->get("val2").toDouble() == 11.0);
    assert(interp.workspace()->get("val3").toDouble() == 0.0);

    // Closure test: workspace snapshot capture
    auto resClos = interp.execute(
        "c = 10;\n"
        "f_clos = @(x) x + c;\n"
        "c = 50;\n"
        "clos_val = f_clos(5);\n"
    );
    assert(resClos.success);
    assert(interp.workspace()->get("clos_val").toDouble() == 15.0);

    // 2. Numerical Quadrature: integral
    auto resInt = interp.execute(
        "q1 = integral(@(x) x.^2, 0, 1);\n"
        "q2 = integral(@(x) sin(x), 0, 3.141592653589793);\n"
    );
    assert(resInt.success);
    assert(std::abs(interp.workspace()->get("q1").toDouble() - 1.0 / 3.0) < 1e-6);
    assert(std::abs(interp.workspace()->get("q2").toDouble() - 2.0) < 1e-6);

    // 3. Root finding: fzero
    auto resZero = interp.execute(
        "r1 = fzero(@(x) x.^2 - 4, 3);\n"
        "r2 = fzero(@(x) cos(x), 1.5);\n"
    );
    assert(resZero.success);
    assert(std::abs(interp.workspace()->get("r1").toDouble() - 2.0) < 1e-6);
    assert(std::abs(interp.workspace()->get("r2").toDouble() - 3.141592653589793 / 2.0) < 1e-6);

    // 4. Equation solvers: solve, linsolve, fsolve
    auto resSolvers = interp.execute(
        "A = [2, 1; 1, 3];\n"
        "b = [5; 5];\n"
        "x_lin = linsolve(A, b);\n"
        "x_solve = solve(A, b);\n"
        "x_scalar = solve(4, 12);\n"
        "x_nonlin = solve(@(x) x.^2 - 9, 2);\n"
        "x_fsolve = fsolve(@(x) x.^3 - 27, 2);\n"
    );
    assert(resSolvers.success);
    auto xLin = interp.workspace()->get("x_lin").toMatrix();
    assert(std::abs(xLin(0, 0) - 2.0) < 1e-8 && std::abs(xLin(1, 0) - 1.0) < 1e-8);
    auto xSolve = interp.workspace()->get("x_solve").toMatrix();
    assert(std::abs(xSolve(0, 0) - 2.0) < 1e-8 && std::abs(xSolve(1, 0) - 1.0) < 1e-8);
    assert(interp.workspace()->get("x_scalar").toDouble() == 3.0);
    assert(std::abs(interp.workspace()->get("x_nonlin").toDouble() - 3.0) < 1e-4);
    assert(std::abs(interp.workspace()->get("x_fsolve").toDouble() - 3.0) < 1e-4);

    // 5. Calculus: diff & gradient
    auto resCalc = interp.execute(
        "d1 = diff([1, 4, 9, 16]);\n"
        "d2 = diff([1, 4, 9, 16], 2);\n"
        "g = gradient([1, 2, 4, 7, 11]);\n"
    );
    assert(resCalc.success);
    auto d1 = interp.workspace()->get("d1").toMatrix();
    assert(d1.cols() == 3 && d1(0, 0) == 3.0 && d1(0, 1) == 5.0 && d1(0, 2) == 7.0);
    auto d2 = interp.workspace()->get("d2").toMatrix();
    assert(d2.cols() == 2 && d2(0, 0) == 2.0 && d2(0, 1) == 2.0);
    auto g = interp.workspace()->get("g").toMatrix();
    assert(g.cols() == 5 && g(0, 0) == 1.0 && g(0, 1) == 1.5 && g(0, 2) == 2.5 && g(0, 3) == 3.5 && g(0, 4) == 4.0);

    // 6. Trapezoidal integration: trapz & cumtrapz
    auto resTrap = interp.execute(
        "tz = trapz([0, 1, 2, 3], [0, 1, 4, 9]);\n"
        "ctz = cumtrapz([0, 1, 2, 3], [0, 1, 4, 9]);\n"
    );
    assert(resTrap.success);
    assert(std::abs(interp.workspace()->get("tz").toDouble() - 9.5) < 1e-8);
    auto ctz = interp.workspace()->get("ctz").toMatrix();
    assert(ctz.cols() == 4 && std::abs(ctz(0, 3) - 9.5) < 1e-8);

    // 7. Unconstrained Optimization: fminsearch
    auto resOpt = interp.execute(
        "opt_x = fminsearch(@(x) (x - 3).^2 + 4, 0);\n"
    );
    assert(resOpt.success);
    assert(std::abs(interp.workspace()->get("opt_x").toDouble() - 3.0) < 1e-4);

    // 8. ODE Solvers: ode45 & ode23
    auto resOde = interp.execute(
        "[t45, y45] = ode45(@(t, y) -2*y, [0, 1], 1);\n"
        "y45_final = y45(end);\n"
        "[t23, y23] = ode23(@(t, y) -2*y, [0, 1], 1);\n"
        "y23_final = y23(end);\n"
        "sol_single = ode45(@(t, y) -2*y, [0, 1], 1);\n"
    );
    assert(resOde.success);
    double expectedOde = std::exp(-2.0);
    assert(std::abs(interp.workspace()->get("y45_final").toDouble() - expectedOde) < 1e-3);
    assert(std::abs(interp.workspace()->get("y23_final").toDouble() - expectedOde) < 1e-2);
    auto solSingle = interp.workspace()->get("sol_single").toMatrix();
    assert(solSingle.cols() == 2 && solSingle.rows() > 5);

    std::cout << "[PASS] Phase 4 (Function Handles, solve, linsolve, fzero, fsolve, integral, diff, gradient, trapz, cumtrapz, fminsearch, ode45, ode23)\n";
}

int main() {
    std::cout << "Running Interpreter unit tests...\n";
    testAssignmentsAndMath();
    testMatrixEvaluation();
    testColonAndSlicing();
    testSubscriptAssignment();
    testControlFlow();
    testUserFunctions();
    testPhase2Interpreter();
    testPhase3GraphicsCommands();
    testPhase4();
    std::cout << "All Interpreter unit tests passed!\n";
    return 0;
}

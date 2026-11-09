# MATForge

**MATForge** is a high-performance MATLAB clone desktop application written in **C++20** and **Qt 6**, designed and configured for **MSVC (Visual Studio 2022)** and **CMake**.

---

## 🌟 Key Features

- **MATLAB-Style Desktop IDE (Qt 6)**:
  - **Command Window**: Interactive REPL console with `>> ` prompt, command history navigation (Up/Down arrow keys), and color-coded output.
  - **Script Editor**: Multi-tab code editor with MATLAB syntax highlighting (`%` comments, keywords, built-ins, strings), line numbers margin, and F5 script execution.
  - **Workspace Browser & Spreadsheet Inspector**: Live variable table showing Variable Name, Value preview, Size, Memory (Bytes), and Class. Double-clicking any variable opens a dedicated **Excel/MATLAB-style spreadsheet editor dialog** for live inspection, editing, and CSV export.
  - **Current Folder Explorer**: Integrated file tree view for quick navigation and one-click script loading.
  - **Command History**: Timestamped log of executed commands with double-click re-run.
  - **Interactive Figure Window (Phase 3)**: High-performance 2D plotting canvas with:
    - Multiple curves with `hold on` / `hold off` and auto-cycling color palettes.
    - Full MATLAB format specifiers: colors (`r`, `g`, `b`, `c`, `m`, `y`, `k`, `w`), line styles (`-`, `--`, `:`, `-.`), and markers (`o`, `+`, `*`, `.`, `x`, `s`, `d`).
    - Multiple plot types: Line plots, Bar plots (`bar(y)`), and Stem plots (`stem(x, y)`).
    - Annotations: `title()`, `xlabel()`, `ylabel()`, `legend()`, and `grid on` / `grid off`.
    - Multi-axis subplots: `subplot(m, n, p)`.
    - Interactive Canvas Tools: Mouse wheel zoom, click-and-drag pan, real-time Data Cursor hover tooltip, and image export.

- **High-Performance Numeric Engine (`matforge_core`) & Eigen 5.0.1 Integration**:
  - Full integration with **Eigen 5.0.1** (header-only linear algebra template library).
  - Multi-dimensional Matrix arithmetic: `+`, `-`, `*` (matrix product), element-wise operations (`.*`, `./`, `.\`, `.^`), and transpose (`'`).
  - Backslash operator `\` (`mldivide`) for linear system solving and least-squares ($A x = b$).
  - Right division `/` (`mrdivide`) for solving $X B = A$.
  - Matrix decompositions & solvers:
    - Inverse: `inv(A)`
    - Moore-Penrose Pseudo-Inverse: `pinv(A)`
    - Matrix Rank & Condition Number: `rank(A)`, `cond(A)`
    - Singular Value Decomposition: `svd(A)` and `[U, S, V] = svd(A)`
    - Eigenvalues & Eigenvectors: `eig(A)` and `[V, D] = eig(A)`
    - QR Decomposition: `qr(A)` and `[Q, R] = qr(A)`
    - LU Decomposition: `lu(A)` and `[L, U] = lu(A)`
    - Cholesky Factorization: `chol(A)`
  - Statistics & Random Number Generators:
    - `rand(m, n)`, `randn(m, n)`, `randi(imax, m, n)`, `rng(seed)`
    - `std(x)`, `var(x)`, `median(x)`, `mean(x)`, `sum(x)`
  - Signal Processing:
    - Fast Fourier Transform: `fft(x)` (Cooley-Tukey radix-2 with DFT fallback)
    - Inverse FFT: `ifft(Y)`
    - 1D Convolution: `conv(u, v)`
    - Complex extraction & magnitude: `real(Y)`, `imag(Y)`, `abs(Y)`
  - Delimited Numeric File I/O:
    - `readmatrix('data.csv')` / `csvread(...)`
    - `writematrix(A, 'data.csv')` / `csvwrite(...)`
  - Static matrix generators: `zeros()`, `ones()`, `eye()`, `linspace()`, `colon` (`:`).
  - Dynamic `Value` type for MATLAB numbers, matrices, strings, and booleans.

- **Interpreter & Parser (`matforge_engine`)**:
  - Full lexer, parser, and Abstract Syntax Tree (AST).
  - Variable assignments (`A = [1, 2; 3, 4]`).
  - Semicolon `;` output suppression (classic MATLAB behavior).
  - Command handling: `clear`, `clc`, `who`, `whos`, `help`, `exit`, `quit`.
  - Built-in math functions: `sin`, `cos`, `tan`, `sqrt`, `exp`, `log`, `abs`, `det`, `trace`, `length`, `size`, `disp`, `plot`.

- **Standalone CLI REPL (`matforge_cli`)**:
  - Lightweight command-line terminal to execute MATLAB scripts and commands directly without launching the GUI.

---

## 📂 Project Architecture

```
MATForge/
├── CMakeLists.txt              # Root CMake configuration (MSVC / C++20 / Qt6)
├── CMakePresets.json           # Native Visual Studio 2022 presets
├── .gitignore                  # Git ignore rules for MSVC, CMake, and Qt
├── README.md                   # Project documentation
├── scripts/
│   ├── install_qt.ps1          # Automated Qt 6 installer for MSVC via aqtinstall
│   └── build_vs2022.ps1        # One-click build script for VS 2022
├── src/
│   ├── core/                   # Core math, Matrix & Value data structures
│   │   ├── include/matforge/core/
│   │   │   ├── Matrix.hpp
│   │   │   ├── Value.hpp
│   │   │   └── MathUtils.hpp
│   │   └── src/
│   ├── engine/                 # Lexer, Parser, AST, Workspace, and Interpreter
│   │   ├── include/matforge/engine/
│   │   │   ├── Token.hpp
│   │   │   ├── Lexer.hpp
│   │   │   ├── Ast.hpp
│   │   │   ├── Parser.hpp
│   │   │   ├── Workspace.hpp
│   │   │   └── Interpreter.hpp
│   │   └── src/
│   ├── gui/                    # Qt 6 MATLAB IDE components
│   │   ├── include/matforge/gui/
│   │   │   ├── MainWindow.hpp
│   │   │   ├── CommandWindow.hpp
│   │   │   ├── CodeEditor.hpp
│   │   │   ├── MatlabHighlighter.hpp
│   │   │   ├── WorkspaceWidget.hpp
│   │   │   ├── FileBrowserWidget.hpp
│   │   │   ├── CommandHistoryWidget.hpp
│   │   │   └── FigureWindow.hpp
│   │   └── src/
│   ├── app/                    # MATForge GUI Desktop Application
│   │   ├── main.cpp
│   │   └── resources/
│   │       ├── style.qss
│   │       └── matforge.qrc
│   └── cli/                    # MATForge Interactive CLI REPL
│       └── main_cli.cpp
└── tests/                      # Unit tests (Matrix & Interpreter)
    ├── test_matrix.cpp
    └── test_interpreter.cpp
```

---

## 🚀 Getting Started

### 1. Prerequisites
- **Visual Studio 2022** with the **Desktop development with C++** workload.
- **CMake 3.20+** (included with VS 2022 or installed globally).
- **Qt 6** (MSVC 2022 64-bit).

---

### 2. Installing Qt 6 for MSVC 2022

If you do not have Qt 6 installed yet, the easiest and fastest way (without creating an account or downloading 40 GB) is via the included automated script:

```powershell
# From the MATForge project directory:
powershell -ExecutionPolicy Bypass -File .\scripts\install_qt.ps1 -QtVersion "6.7.2" -InstallDir "C:\Qt"
```

> **Note**: This uses `aqtinstall` (Another Qt Installer) to download precompiled official binaries directly to `C:\Qt\6.7.2\msvc2022_64`.

---

### 3. Building with Visual Studio 2022 (MSVC)

#### Option A: Using the PowerShell Build Script (Recommended)
```powershell
# Configure, build with MSVC, and run unit tests:
.\scripts\build_vs2022.ps1 -Config Debug -RunTests

# Or open the generated solution in Visual Studio 2022:
.\scripts\build_vs2022.ps1 -OpenSolution
```

#### Option B: Using CMake from the Command Line
```powershell
# 1. Configure for Visual Studio 2022 x64 (pass your Qt path if building GUI)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/msvc2022_64"

# 2. Build Debug configuration
cmake --build build --config Debug

# 3. Run unit tests
cd build; ctest -C Debug --output-on-failure
```

#### Option C: Opening directly in Visual Studio 2022
1. Launch **Visual Studio 2022**.
2. Select **File > Open > Folder...** and choose the `MATForge` directory.
3. Visual Studio will automatically detect `CMakePresets.json` and configure using MSVC.
4. Select the target (`matforge_app.exe` or `matforge_cli.exe`) and press **F5** to run.

---

## 🧪 Running Unit Tests

Unit tests verify matrix operations, linear algebra, parser semantics, and interpreter execution:

```powershell
cd build
ctest -C Debug --output-on-failure
```

---

## 💻 Sample Commands

Launch `matforge_cli.exe` or `matforge_app.exe`:

### Matrix Arithmetic & Slicing
```matlab
>> A = [1, 2, 3; 4, 5, 6; 7, 8, 9]
>> corner = A(2:end, end-1:end)
corner =
       5.0000    6.0000
       8.0000    9.0000

>> A(1, :) = [10, 20, 30];   % Row assignment
>> A(4, 4) = 99;             % Auto-expansion to 4x4 matrix
```

### Colon Ranges
```matlab
>> v = 1:5;                  % [1, 2, 3, 4, 5]
>> t = 0:0.1:10;             % Step range
>> rev = 10:-2:0;            % Decrementing range
```

### Control Flow (Loops & Conditionals)
```matlab
total = 0;
for i = 1:10
    total = total + i;
end
total

val = 7;
if val > 10
    out = 1;
elseif val > 5
    out = 2;
else
    out = 3;
end
```

### User-Defined Functions
```matlab
function y = square(x)
    y = x * x;
end

val = square(6)              % Returns 36.0000
```

### Plotting & Figure Commands
```matlab
t = linspace(0, 6.28, 100);
y = sin(t);
plot(t, y)

clf                          % Clears current plot
close                        % Closes the figure window
```

### Linear Algebra & Solvers (Phase 2)
```matlab
% Linear System Solve Ax = b
A = [4, 7; 2, 6];
b = [1; 2];
x = A \ b                    % mldivide solve

% Multi-Assignment Decompositions
[U, S, V] = svd(A)           % Singular Value Decomposition
[Q, R] = qr(A)               % QR Factorization
[L, U_lu] = lu(A)            % LU Factorization
[V, D] = eig(A)              % Eigenvectors & Eigenvalues
R_chol = chol([4, 2; 2, 5])  % Cholesky Decomposition

% Matrix Inverses, Rank, and Condition Number
invA = inv(A)
pinvA = pinv(A)              % Moore-Penrose Pseudo-Inverse
r = rank(A)
c = cond(A)
```

### Statistics & Random Numbers
```matlab
rng(42)                      % Set seed
R = rand(3, 3)               % Uniform random in [0, 1)
Rn = randn(3, 3)             % Standard normal N(0, 1)
Ri = randi(10, 4, 4)         % Discrete uniform integers in [1, 10]

s = std(1:10)
v = var(1:10)
m = median([1, 10, 3, 7, 5])
```

### Signal Processing & Transforms
```matlab
% 1D Convolution
w = conv([1, 2, 3], [4, 5])  % Returns [4, 13, 22, 15]

% Fast Fourier Transform & Inverse
sig = [1, 2, 3, 4];
Y = fft(sig)                 % Returns N x 2 complex matrix [real, imag]
mag = abs(Y)                 % Complex magnitude
recon = ifft(Y)              % Reconstructed signal
```

### Delimited File I/O
```matlab
% Export matrix to CSV
writematrix([1, 2, 3; 4, 5, 6], 'dataset.csv');

% Import matrix from CSV
data = readmatrix('dataset.csv');
```

### Rich 2D Graphics & Plotting (Phase 3)
```matlab
% Multiple Curves with hold on and line styles
x = linspace(0, 2*pi, 100);
y1 = sin(x);
y2 = cos(x);

plot(x, y1, 'b-')             % Solid blue line
hold on                       % Retain current plot
plot(x, y2, 'r--*')           % Dashed red line with asterisk markers
grid on                       % Enable grid lines

title('Harmonic Oscillations')
xlabel('Time (s)')
ylabel('Amplitude')
legend('sin(x)', 'cos(x)')

% Discrete and Bar Plots
subplot(2, 1, 1);
bar([10, 25, 18, 30, 15]);   % Bar chart
title('Monthly Revenue')

subplot(2, 1, 2);
stem(0:0.1:1, exp(-(0:0.1:1))); % Discrete stem plot
title('Damped Exponential')

% Interactive Tools on Canvas:
% - Mouse Wheel: Zoom in/out at cursor
% - Click & Drag: Pan axes
% - Hover: Data Cursor showing exact (X, Y) coordinates
% - Double-Click: Reset View / Auto-scale
```

### Variable Spreadsheet Inspector (Phase 3)
- **Live Spreadsheet Editor**: Double-click any variable in the **Workspace Browser** dock panel to open an Excel/MATLAB-style tabular editor dialog.
- **Real-Time Synchronous Editing**: Modifying any matrix cell immediately writes back to the active workspace.
- **Dynamic Array Expansion**: Add rows or columns directly with the click of a button.
- **Export to CSV**: One-click dataset export.

---

## 📄 License
This project is open-source and released under the MIT License.

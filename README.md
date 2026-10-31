# MATForge

**MATForge** is a high-performance MATLAB clone desktop application written in **C++20** and **Qt 6**, designed and configured for **MSVC (Visual Studio 2022)** and **CMake**.

---

## 🌟 Key Features

- **MATLAB-Style Desktop IDE (Qt 6)**:
  - **Command Window**: Interactive REPL console with `>> ` prompt, command history navigation (Up/Down arrow keys), and color-coded output.
  - **Script Editor**: Multi-tab code editor with MATLAB syntax highlighting (`%` comments, keywords, built-ins, strings), line numbers margin, and F5 script execution.
  - **Workspace Browser**: Live variable table showing Variable Name, Value preview, Size (`2x2 double`), Memory (Bytes), and Class (`double`, `char`, `logical`).
  - **Current Folder Explorer**: Integrated file tree view for quick navigation and one-click script loading.
  - **Command History**: Timestamped log of executed commands with double-click re-run.
  - **Figure Window**: 2D plotting canvas with grid lines, axis tick scaling, custom labels, and image export (`plot(x, y)`).

- **High-Performance Numeric Engine (`matforge_core`)**:
  - Multi-dimensional Matrix arithmetic: `+`, `-`, `*` (matrix dot product), element-wise operations (`.*`, `./`, `.^`), and transpose (`'`).
  - Static matrix generators: `zeros()`, `ones()`, `eye()`, `linspace()`.
  - Linear algebra routines: determinant, trace, sum, mean, min, max.
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

```matlab
>> A = [1, 2; 3, 4]
A =
       1.0000    2.0000
       3.0000    4.0000

>> b = [5; 6];
>> x = A * b
x =
      17.0000
      39.0000

>> whos
  Name      Size         Bytes  Class
  ====      ====         =====  =====
  A         2x2          48     double
  b         2x1          32     double
  x         2x1          32     double

>> t = linspace(0, 6.28, 50);
>> y = sin(t);
>> plot(t, y)
```

---

## 📄 License
This project is open-source and released under the MIT License.

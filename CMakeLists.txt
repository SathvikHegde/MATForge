cmake_minimum_required(VERSION 3.20)

project(MATForge 
    VERSION 0.1.0
    DESCRIPTION "A high-performance MATLAB clone built with C++20 and Qt"
    LANGUAGES CXX
)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling (e.g., clangd, VS Code, VS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(MATFORGE_BUILD_GUI "Build MATForge Qt GUI desktop IDE" ON)
option(MATFORGE_BUILD_CLI "Build MATForge Command-Line REPL" ON)
option(MATFORGE_BUILD_TESTS "Build MATForge Unit Tests" ON)

# MSVC compiler options and warnings
if(MSVC)
    add_compile_options(
        /W4          # Baseline warning level 4
        /permissive- # Strict standards conformance
        /utf-8       # Set Source and Executable character sets to UTF-8
        /EHsc        # Standard C++ stack unwinding
        /MP          # Multi-processor compilation
        /DNOMINMAX   # Prevent Windows.h from defining min and max macros
    )
endif()

# Qt 6 Configuration for GUI
if(MATFORGE_BUILD_GUI)
    # Search for Qt 6 Core, Gui, and Widgets
    find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)

    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
        message(STATUS "--> Found Qt6 version: ${Qt6_VERSION}")
        message(STATUS "--> Qt6 Core directory: ${Qt6Core_DIR}")
    else()
        message(WARNING 
            "Qt6 was not found on your system.\n"
            "   GUI application target 'matforge_app' will be skipped for now.\n"
            "   The core math engine, parser, and CLI REPL ('matforge_cli') will still build.\n"
            "   To enable the GUI:\n"
            "     1. Run 'scripts/install_qt.ps1' (installs Qt 6 for MSVC via aqtinstall),\n"
            "     2. Reconfigure CMake passing: -DCMAKE_PREFIX_PATH=\"C:/Qt/<version>/msvc2022_64\""
        )
        set(MATFORGE_BUILD_GUI OFF)
    endif()
endif()

# Add Subdirectories
add_subdirectory(src/core)
add_subdirectory(src/engine)

if(MATFORGE_BUILD_GUI)
    add_subdirectory(src/gui)
    add_subdirectory(src/app)
endif()

if(MATFORGE_BUILD_CLI)
    add_subdirectory(src/cli)
endif()

if(MATFORGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print Project Summary
message(STATUS "--------------------------------------------------------")
message(STATUS "MATForge configuration summary:")
message(STATUS "  - Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  - C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  - Build GUI:         ${MATFORGE_BUILD_GUI}")
message(STATUS "  - Build CLI REPL:    ${MATFORGE_BUILD_CLI}")
message(STATUS "  - Build Unit Tests:  ${MATFORGE_BUILD_TESTS}")
message(STATUS "--------------------------------------------------------")

#!/usr/bin/env bash
#
# MATForge Linux / WSL Build Script
# Configures, builds, and optionally tests MATForge using CMake, GCC/Clang, and Ninja/Make.
#

set -euo pipefail

# Colors
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Defaults
BUILD_TYPE="Debug"
RUN_TESTS=false
CLEAN=false
QT_PATH=""
BUILD_GUI=true
GENERATOR=""
BUILD_DIR=""

print_usage() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS]

Options:
    -c, --config <type>       Build type: Debug (default) or Release
    -t, --tests               Run unit tests after build
    --clean                   Remove existing build directory before building
    --qt-path <path>          Path to Qt 6 cmake directory or prefix
    --no-gui                  Disable Qt 6 GUI build (CLI only)
    -g, --generator <gen>     CMake generator: 'Ninja' or 'Unix Makefiles'
    -b, --build-dir <dir>     Custom build directory (default: build/linux-<config>)
    -h, --help                Show this help message

Examples:
    ./scripts/build_linux.sh -t
    ./scripts/build_linux.sh --config Release -t
    ./scripts/build_linux.sh --no-gui
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--config)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -t|--tests)
            RUN_TESTS=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --qt-path)
            QT_PATH="$2"
            shift 2
            ;;
        --no-gui)
            BUILD_GUI=false
            shift
            ;;
        -g|--generator)
            GENERATOR="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo -e "${RED}[!] Unknown option: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

echo -e "${CYAN}==========================================================${NC}"
echo -e "${CYAN}  MATForge Build Script (Linux / WSL)${NC}"
echo -e "${CYAN}==========================================================${NC}"

# Detect Generator if not specified
if [[ -z "${GENERATOR}" ]]; then
    if command -v ninja >/dev/null 2>&1; then
        GENERATOR="Ninja"
    else
        GENERATOR="Unix Makefiles"
    fi
fi

# Set default build dir
if [[ -z "${BUILD_DIR}" ]]; then
    BUILD_DIR="${PROJECT_DIR}/build/linux-${BUILD_TYPE,,}"
fi

if [[ "${CLEAN}" == true && -d "${BUILD_DIR}" ]]; then
    echo -e "${YELLOW}[*] Cleaning build directory: ${BUILD_DIR}${NC}"
    rm -rf "${BUILD_DIR}"
fi

echo -e "${YELLOW}[*] Build Type:  ${BUILD_TYPE}${NC}"
echo -e "${YELLOW}[*] Generator:   ${GENERATOR}${NC}"
echo -e "${YELLOW}[*] Build Dir:   ${BUILD_DIR}${NC}"

# Prepare CMake arguments
CMAKE_ARGS=(
    "-B" "${BUILD_DIR}"
    "-S" "${PROJECT_DIR}"
    "-G" "${GENERATOR}"
    "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    "-DMATFORGE_BUILD_TESTS=ON"
)

if [[ "${BUILD_GUI}" == false ]]; then
    CMAKE_ARGS+=("-DMATFORGE_BUILD_GUI=OFF")
fi

# Qt detection
if [[ -n "${QT_PATH}" ]]; then
    CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=${QT_PATH}")
else
    # Check common Qt6 install paths on Linux
    CANDIDATE_QT_PATHS=(
        "/usr/lib/x86_64-linux-gnu/cmake/Qt6"
        "/usr/lib/cmake/Qt6"
        "${HOME}/Qt/6.7.2/gcc_64"
        "${HOME}/Qt/6.6.3/gcc_64"
        "${HOME}/Qt/6.8.0/gcc_64"
        "/opt/Qt/6.7.2/gcc_64"
        "/opt/Qt/6.6.3/gcc_64"
    )
    for cand in "${CANDIDATE_QT_PATHS[@]}"; do
        if [[ -d "${cand}" ]]; then
            echo -e "${GREEN}[+] Auto-detected Qt installation at: ${cand}${NC}"
            CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=${cand}")
            break
        fi
    done
fi

echo -e "${YELLOW}[*] Configuring CMake project...${NC}"
cmake "${CMAKE_ARGS[@]}"

echo -e "${YELLOW}[*] Building MATForge (${BUILD_TYPE})...${NC}"
cmake --build "${BUILD_DIR}" -- -j"$(nproc 2>/dev/null || echo 4)"

echo -e "${GREEN}==========================================================${NC}"
echo -e "${GREEN}  Build completed successfully!${NC}"
echo -e "${GREEN}  Binaries located in: ${BUILD_DIR}/bin${NC}"
echo -e "${GREEN}==========================================================${NC}"

if [[ "${RUN_TESTS}" == true ]]; then
    echo -e "${YELLOW}[*] Running Unit Tests via ctest...${NC}"
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
fi

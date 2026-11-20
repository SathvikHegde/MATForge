#!/usr/bin/env bash
#
# MATForge Qt 6 Automated / Guided Installer for Linux & WSL
# Installs Qt 6 development libraries for building MATForge GUI ('matforge_app').
#

set -euo pipefail

CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${CYAN}==========================================================${NC}"
echo -e "${CYAN}  MATForge Qt 6 Installer & Setup for Linux / WSL${NC}"
echo -e "${CYAN}==========================================================${NC}"

# 1. Check if Qt6 is already installed
echo -e "${YELLOW}[*] Checking for existing Qt 6 installation...${NC}"
if command -v qmake6 >/dev/null 2>&1 || [ -d "/usr/lib/x86_64-linux-gnu/cmake/Qt6" ] || [ -d "/usr/lib/cmake/Qt6" ]; then
    echo -e "${GREEN}[+] Qt 6 is already installed on your system!${NC}"
    echo -e "You can build MATForge GUI directly:"
    echo -e "  ./scripts/build_linux.sh -t"
    exit 0
fi

# 2. Check distribution
DISTRO=""
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO="${ID:-linux}"
fi

echo -e "[*] Detected distribution: ${DISTRO}"

install_apt() {
    echo -e "${YELLOW}[*] Ubuntu / Debian detected. Installing via apt...${NC}"
    echo -e "Running: sudo apt-get update && sudo apt-get install -y qt6-base-dev libgl1-mesa-dev"
    sudo apt-get update
    sudo apt-get install -y qt6-base-dev libgl1-mesa-dev
}

install_dnf() {
    echo -e "${YELLOW}[*] Fedora / RHEL detected. Installing via dnf...${NC}"
    echo -e "Running: sudo dnf install -y qt6-qtbase-devel mesa-libGL-devel"
    sudo dnf install -y qt6-qtbase-devel mesa-libGL-devel
}

install_pacman() {
    echo -e "${YELLOW}[*] Arch Linux detected. Installing via pacman...${NC}"
    echo -e "Running: sudo pacman -S --needed qt6-base mesa"
    sudo pacman -S --needed qt6-base mesa
}

# 3. Attempt system package install if run with privileges or ask user
if [ "${EUID}" -eq 0 ] || sudo -n true 2>/dev/null; then
    case "${DISTRO}" in
        ubuntu|debian|pop|linuxmint)
            install_apt
            ;;
        fedora|rhel|centos)
            install_dnf
            ;;
        arch|manjaro)
            install_pacman
            ;;
        *)
            echo -e "${YELLOW}[!] Unknown distro. Attempting apt-get...${NC}"
            install_apt || true
            ;;
    esac
else
    echo -e "${YELLOW}[*] sudo privileges required for system package manager.${NC}"
    echo -e "Please run one of the following commands depending on your distribution:\n"
    case "${DISTRO}" in
        ubuntu|debian|pop|linuxmint)
            echo -e "  ${GREEN}sudo apt-get update && sudo apt-get install -y qt6-base-dev libgl1-mesa-dev${NC}\n"
            ;;
        fedora|rhel|centos)
            echo -e "  ${GREEN}sudo dnf install -y qt6-qtbase-devel mesa-libGL-devel${NC}\n"
            ;;
        arch|manjaro)
            echo -e "  ${GREEN}sudo pacman -S --needed qt6-base mesa${NC}\n"
            ;;
        *)
            echo -e "  Ubuntu/Debian:  ${GREEN}sudo apt install -y qt6-base-dev libgl1-mesa-dev${NC}"
            echo -e "  Fedora:         ${GREEN}sudo dnf install -y qt6-qtbase-devel mesa-libGL-devel${NC}"
            echo -e "  Arch:           ${GREEN}sudo pacman -S --needed qt6-base mesa${NC}\n"
            ;;
    esac

    # Offer non-root aqtinstall option if python3 is available
    if command -v python3 >/dev/null 2>&1; then
        echo -e "${CYAN}Alternative: Install Qt 6 locally into ~/Qt (No root/sudo required):${NC}"
        echo -e "  pip install --user aqtinstall"
        echo -e "  python3 -m aqt install-qt linux desktop 6.7.2 gcc_64 --outputdir ~/Qt -m qtbase"
        echo -e "  ./scripts/build_linux.sh --qt-path ~/Qt/6.7.2/gcc_64\n"
    fi
fi

# 4. WSLg / Display configuration note
echo -e "${CYAN}==========================================================${NC}"
echo -e "${CYAN}  WSL Display / GUI Notes:${NC}"
echo -e "${CYAN}==========================================================${NC}"
echo -e "  - Windows 11 / WSL 2 includes WSLg (Wayland/X11) built-in."
echo -e "    GUI apps will open native Windows desktop windows automatically."
echo -e "  - For Windows 10 or older WSL 2 setups without WSLg:"
echo -e "    Install an X Server on Windows (e.g. VcXsrv) and run:"
echo -e "    export DISPLAY=\$(ip route | awk '/default/ { print \$3 }'):0"
echo -e "==========================================================\n"

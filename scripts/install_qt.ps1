<#
.SYNOPSIS
    Automated Qt 6 installer for MSVC 2022 on Windows using aqtinstall.
.DESCRIPTION
    This script downloads and installs official Qt 6 pre-built binaries for Visual Studio 2022 (x64)
    without requiring a Qt account or manual GUI installation.
#>

param(
    [string]$QtVersion = "6.7.2",
    [string]$InstallDir = "C:\Qt"
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  MATForge Qt 6 Automated Installer for MSVC 2022" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. Check Python
$pythonCmd = Get-Command python -ErrorAction SilentlyContinue
if (-not $pythonCmd) {
    Write-Error "Python 3 is required to run aqtinstall. Please ensure Python is installed and added to PATH."
    exit 1
}

Write-Host "[+] Detected Python: $($pythonCmd.Source)" -ForegroundColor Green

# 2. Check / Install aqtinstall
Write-Host "[*] Checking for aqtinstall..." -ForegroundColor Yellow
$aqtCheck = python -m pip show aqtinstall 2>$null
if (-not $aqtCheck) {
    Write-Host "[*] Installing aqtinstall via pip..." -ForegroundColor Yellow
    python -m pip install --upgrade aqtinstall
} else {
    Write-Host "[+] aqtinstall is already installed." -ForegroundColor Green
}

# 3. Create destination directory
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

$targetPath = Join-Path $InstallDir "$QtVersion\msvc2022_64"
Write-Host "[*] Installing Qt $QtVersion (win64_msvc2022_64) into: $InstallDir ..." -ForegroundColor Yellow
Write-Host "    This may take a few minutes depending on network speed." -ForegroundColor DarkGray

python -m aqt install-qt windows desktop $QtVersion win64_msvc2022_64 --outputdir $InstallDir -m qtbase

if (Test-Path $targetPath) {
    Write-Host "==========================================================" -ForegroundColor Green
    Write-Host "  Qt $QtVersion installed successfully!" -ForegroundColor Green
    Write-Host "  Location: $targetPath" -ForegroundColor Green
    Write-Host "==========================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "To configure and build MATForge with GUI enabled in VS 2022:" -ForegroundColor Cyan
    Write-Host "  cmake -B build -G `"Visual Studio 17 2022`" -A x64 -DCMAKE_PREFIX_PATH=`"$targetPath`"" -ForegroundColor White
    Write-Host "  cmake --build build --config Debug" -ForegroundColor White
} else {
    Write-Host "[!] Could not verify target directory $targetPath. Check the output above for any errors." -ForegroundColor Red
}


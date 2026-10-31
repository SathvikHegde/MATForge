<#
.SYNOPSIS
    Configures and builds MATForge with MSVC (Visual Studio 2022) using CMake.
#>

param(
    [string]$Config = "Debug",
    [string]$QtPath = "",
    [switch]$RunTests,
    [switch]$OpenSolution
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectDir = Split-Path -Parent $scriptDir
$buildDir = Join-Path $projectDir "build\vs2022"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  MATForge Build Script (Visual Studio 2022 / MSVC)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$cmakeArgs = @(
    "-B", $buildDir,
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DMATFORGE_BUILD_TESTS=ON"
)

# Detect Qt if path passed or common locations exist
if ($QtPath -ne "") {
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtPath"
} else {
    $candidateQtPaths = @(
        "D:\Qt\6.7.2\msvc2019_64",
        "D:\Qt\6.7.2\msvc2022_64",
        "C:\Qt\6.7.2\msvc2019_64",
        "C:\Qt\6.7.2\msvc2022_64",
        "C:\Qt\6.6.3\msvc2022_64",
        "C:\Qt\6.5.3\msvc2022_64",
        "C:\Qt\6.8.0\msvc2022_64"
    )
    foreach ($cand in $candidateQtPaths) {
        if (Test-Path $cand) {
            Write-Host "[+] Auto-detected Qt installation at: $cand" -ForegroundColor Green
            $cmakeArgs += "-DCMAKE_PREFIX_PATH=$cand"
            break
        }
    }
}

Write-Host "[*] Configuring CMake project..." -ForegroundColor Yellow
& cmake $cmakeArgs $projectDir

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
    exit $LASTEXITCODE
}

Write-Host "[*] Building MATForge ($Config)..." -ForegroundColor Yellow
& cmake --build $buildDir --config $Config

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit $LASTEXITCODE
}

Write-Host "==========================================================" -ForegroundColor Green
Write-Host "  Build completed successfully!" -ForegroundColor Green
Write-Host "  Binaries located in: $buildDir\bin\$Config" -ForegroundColor Green
Write-Host "==========================================================" -ForegroundColor Green

if ($RunTests) {
    Write-Host "[*] Running Unit Tests via ctest..." -ForegroundColor Yellow
    Push-Location $buildDir
    & ctest -C $Config --output-on-failure
    Pop-Location
}

if ($OpenSolution) {
    $slnPath = Join-Path $buildDir "MATForge.sln"
    if (Test-Path $slnPath) {
        Write-Host "[*] Opening Visual Studio 2022 solution: $slnPath" -ForegroundColor Cyan
        Start-Process $slnPath
    }
}


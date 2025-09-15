# Prometheus Windows Build Script (PowerShell)
param(
    [string]$BuildType = "Debug",
    [switch]$Run
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Prometheus Windows Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Function to find Visual Studio installation
function Find-VisualStudio {
    $vsPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
    )
    
    foreach ($path in $vsPaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    return $null
}

# Check if cl.exe is available
try {
    $null = Get-Command cl.exe -ErrorAction Stop
    Write-Host "✓ Visual Studio C++ compiler found in PATH" -ForegroundColor Green
} catch {
    Write-Host "Visual Studio C++ compiler not found in PATH" -ForegroundColor Yellow
    Write-Host "Attempting to locate and setup Visual Studio environment..." -ForegroundColor Yellow
    
    $vsPath = Find-VisualStudio
    if ($null -eq $vsPath) {
        Write-Host "ERROR: Could not find Visual Studio installation." -ForegroundColor Red
        Write-Host "Please install Visual Studio 2019 or 2022 with C++ support." -ForegroundColor Red
        Write-Host "Or manually run vcvarsall.bat before running this script." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green
    Write-Host "Note: You need to run vcvarsall.bat manually before using this PowerShell script." -ForegroundColor Yellow
    Write-Host "Or use the .bat version of this script which handles it automatically." -ForegroundColor Yellow
    exit 1
}

# Check for required tools
Write-Host "Checking for required tools..." -ForegroundColor Yellow

try {
    $null = Get-Command cmake -ErrorAction Stop
    Write-Host "✓ CMake found" -ForegroundColor Green
} catch {
    Write-Host "ERROR: CMake not found in PATH" -ForegroundColor Red
    Write-Host "Please install CMake and add it to your PATH" -ForegroundColor Red
    exit 1
}

try {
    $null = Get-Command ninja -ErrorAction Stop
    Write-Host "✓ Ninja found" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Ninja not found in PATH" -ForegroundColor Red
    Write-Host "Please install Ninja build system and add it to your PATH" -ForegroundColor Red
    Write-Host "You can download it from: https://github.com/ninja-build/ninja/releases" -ForegroundColor Red
    exit 1
}

# Check for Vulkan SDK
if (-not $env:VULKAN_SDK) {
    Write-Host "ERROR: VULKAN_SDK environment variable not set" -ForegroundColor Red
    Write-Host "Please install the Vulkan SDK from: https://vulkan.lunarg.com/" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Vulkan SDK found at: $env:VULKAN_SDK" -ForegroundColor Green

Write-Host "Build type: $BuildType" -ForegroundColor Cyan

# Create build directory
$buildDir = "build_windows_$BuildType"
if (Test-Path $buildDir) {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Path $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir | Out-Null
Set-Location $buildDir

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Configuring with CMake..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Configure with CMake using Ninja and MSVC
$cmakeArgs = @(
    "-G", "Ninja",
    "-DCMAKE_C_COMPILER=cl.exe",
    "-DCMAKE_CXX_COMPILER=cl.exe",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ".."
)

& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building with Ninja..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

& ninja

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "========================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Executable location: $buildDir\client\prometheus_client.exe" -ForegroundColor Green
Write-Host "To run the application, execute:" -ForegroundColor Green
Write-Host "  cd $buildDir\client" -ForegroundColor Green
Write-Host "  .\prometheus_client.exe" -ForegroundColor Green

Set-Location ..

if ($Run) {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Running the application..." -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Set-Location "$buildDir\client"
    & .\prometheus_client.exe
    Set-Location ..\..
}
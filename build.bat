@echo off
setlocal

echo ========================================
echo Prometheus CAD - Quick Build Script
echo ========================================
echo.
echo This script uses the optimized Ninja build system.
echo For Visual Studio builds, use build-debug.bat instead.
echo.

:: Check if user wants Release build
if /I "%~1"=="Release" (
    echo Building Release configuration...
    call build-ninja.bat Release
) else (
    echo Building Debug configuration...
    call build-ninja.bat Debug
)

if errorlevel 1 (
    echo Build failed. See output above for details.
    exit /b 1
)

echo.
echo ========================================
echo Build completed! All DLLs have been
echo automatically copied to the executable folder.
echo ========================================

endlocal
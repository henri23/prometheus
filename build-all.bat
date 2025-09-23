@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Prometheus CAD - Universal Build Script
echo ========================================
echo.
echo Available build options:
echo   1. Ninja Debug (Fast, recommended for development)
echo   2. Ninja Release (Fast, optimized)
echo   3. Visual Studio Debug (IDE integration)
echo   4. Visual Studio Release (IDE integration)
echo.

:: Parse command line arguments
if /I "%~1"=="ninja-debug" goto :ninja_debug
if /I "%~1"=="ninja-release" goto :ninja_release
if /I "%~1"=="vs-debug" goto :vs_debug
if /I "%~1"=="vs-release" goto :vs_release

:: Interactive mode
echo Choose your build type:
echo [1] Ninja Debug
echo [2] Ninja Release
echo [3] Visual Studio Debug
echo [4] Visual Studio Release
echo.
set /p choice="Enter choice (1-4): "

if "%choice%"=="1" goto :ninja_debug
if "%choice%"=="2" goto :ninja_release
if "%choice%"=="3" goto :vs_debug
if "%choice%"=="4" goto :vs_release

echo Invalid choice. Defaulting to Ninja Debug.

:ninja_debug
echo.
echo Building with Ninja (Debug)...
call build-ninja.bat Debug
goto :end

:ninja_release
echo.
echo Building with Ninja (Release)...
call build-ninja.bat Release
goto :end

:vs_debug
echo.
echo Building with Visual Studio (Debug)...
call build-debug.bat
goto :end

:vs_release
echo.
echo Building with Visual Studio (Release)...
:: Check if release VS script exists
if exist "build-release.bat" (
    call build-release.bat
) else (
    echo [INFO] Creating Release build with Visual Studio...
    if not exist "build" mkdir build
    cd build
    cmake -G "Visual Studio 17 2022" -A x64 ..
    if errorlevel 1 goto :error

    cmake --build . --config Release --parallel
    if errorlevel 1 goto :error

    :: Copy DLLs for Release build
    set "CLIENT_DIR=client\Release"
    if exist "%CLIENT_DIR%\prometheus_client.exe" (
        echo Copying DLLs to Release folder...
        if exist "core\Release\prometheus_core.dll" (
            copy /Y "core\Release\prometheus_core.dll" "%CLIENT_DIR%\" >nul
            echo [OK] Copied prometheus_core.dll
        )

        :: Try to find SDL3.dll
        if exist "external\SDL3\Release\SDL3.dll" (
            copy /Y "external\SDL3\Release\SDL3.dll" "%CLIENT_DIR%\" >nul
            echo [OK] Copied SDL3.dll
        ) else if exist "external\SDL3\SDL3.dll" (
            copy /Y "external\SDL3\SDL3.dll" "%CLIENT_DIR%\" >nul
            echo [OK] Copied SDL3.dll
        )

        echo Release build with DLLs completed!
        echo Executable: build\%CLIENT_DIR%\prometheus_client.exe
    )
    cd ..
)
goto :end

:error
echo Build failed!
exit /b 1

:end
if errorlevel 1 (
    echo Build failed. Check output above for details.
    exit /b 1
) else (
    echo.
    echo ========================================
    echo Build completed successfully!
    echo All required DLLs have been copied.
    echo You can run the executable directly.
    echo ========================================
)

endlocal
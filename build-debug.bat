@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo Building Prometheus CAD for Windows (Debug)
echo ===============================================

:: Check if we're in the correct directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    exit /b 1
)

:: Check for Vulkan SDK
if not defined VULKAN_SDK (
    echo Error: VULKAN_SDK environment variable not set.
    echo Please install the Vulkan SDK and ensure VULKAN_SDK is set.
    exit /b 1
)

echo Using Vulkan SDK: %VULKAN_SDK%

:: Initialize submodules if needed
echo Checking git submodules...
git submodule update --init --recursive
if errorlevel 1 (
    echo Warning: Failed to update git submodules. Continuing anyway...
)

:: Create build directory
if not exist "build" mkdir build
cd build

:: Configure CMake
echo Configuring with CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..
if errorlevel 1 (
    echo Error: CMake configuration failed.
    exit /b 1
)

:: Build the project
echo Building (Debug)...
cmake --build . --config Debug --parallel
if errorlevel 1 (
    echo Error: Build failed.
    exit /b 1
)

:: Copy DLLs to client directory
echo ===============================================
echo Copying runtime DLLs to client folder...
echo ===============================================

set "CLIENT_DIR=client\Debug"

if exist "%CLIENT_DIR%\prometheus_client.exe" (
    :: Copy prometheus_core.dll
    if exist "core\Debug\prometheus_core.dll" (
        copy /Y "core\Debug\prometheus_core.dll" "%CLIENT_DIR%\" >nul
        echo [OK] Copied prometheus_core.dll
    ) else (
        echo [WARNING] prometheus_core.dll not found in core\Debug\
    )

    :: Copy SDL3.dll - try multiple locations
    set "SDL_COPIED=false"
    if exist "external\SDL3\Debug\SDL3.dll" (
        copy /Y "external\SDL3\Debug\SDL3.dll" "%CLIENT_DIR%\" >nul
        echo [OK] Copied SDL3.dll from external\SDL3\Debug
        set "SDL_COPIED=true"
    ) else if exist "external\SDL3\SDL3.dll" (
        copy /Y "external\SDL3\SDL3.dll" "%CLIENT_DIR%\" >nul
        echo [OK] Copied SDL3.dll from external\SDL3
        set "SDL_COPIED=true"
    ) else if exist "Debug\SDL3.dll" (
        copy /Y "Debug\SDL3.dll" "%CLIENT_DIR%\" >nul
        echo [OK] Copied SDL3.dll from Debug
        set "SDL_COPIED=true"
    )

    if "%SDL_COPIED%"=="false" (
        echo [WARNING] SDL3.dll not found - application may fail to start
    )

    :: Copy Vulkan validation layer for debug builds
    if exist "%VULKAN_SDK%\Bin\VkLayer_khronos_validation.dll" (
        copy /Y "%VULKAN_SDK%\Bin\VkLayer_khronos_validation.dll" "%CLIENT_DIR%\" >nul
        echo [OK] Copied Vulkan validation layer
    )

    echo.
    echo ===============================================
    echo Debug build completed successfully!
    echo.
    echo Executable location: build\%CLIENT_DIR%\prometheus_client.exe
    echo All required DLLs have been copied to the executable directory.
    echo.
    echo You can now run the application directly:
    echo   cd build\%CLIENT_DIR%
    echo   prometheus_client.exe
    echo ===============================================
) else (
    echo [ERROR] prometheus_client.exe not found in %CLIENT_DIR%
    echo Build may have failed.
)

cd ..
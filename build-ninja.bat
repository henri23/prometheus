@echo off
setlocal enabledelayedexpansion

echo PROMETHEUS BUILD SYSTEM
echo =======================
echo.

:: Parse arguments
set LINKING_MODE=STATIC
if "%~1"=="--dynamic" set LINKING_MODE=DYNAMIC

:: Hardcoded vcvarsall.bat path
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

:: Setup MSVC if cl.exe not found
where cl >nul 2>&1
if errorlevel 1 (
    if not exist %VCVARS% (
        echo ERROR: Could not find vcvarsall.bat at %VCVARS%
        exit /b 1
    )
    echo Setting up MSVC environment...
    call %VCVARS% x64
)

:: Check required tools
for %%T in (cmake ninja cl) do (
    where %%T >nul 2>&1
    if errorlevel 1 (
        echo ERROR: Missing required tool: %%T
        exit /b 1
    )
)

echo All required tools are available.
echo.

:: Linking flags
if "%LINKING_MODE%"=="STATIC" (
    set CMAKE_LINKING_FLAG=-DPROMETHEUS_STATIC_LINKING=ON
) else (
    set CMAKE_LINKING_FLAG=-DPROMETHEUS_STATIC_LINKING=OFF
)

:: Ensure bin directory
if not exist bin mkdir bin

:: Configure with CMake
echo Configuring project with CMake...
cmake -G Ninja ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON ^
    %CMAKE_LINKING_FLAG% ^
    -B bin ^
    .
if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

:: Build
cd bin
echo Building prometheus_client...
ninja prometheus_client
if errorlevel 1 (
    echo ERROR: Build failed.
    exit /b 1
)

echo Build completed successfully.
echo Launching prometheus_client...
client\prometheus_client.exe
cd ..

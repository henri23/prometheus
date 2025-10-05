@echo off
setlocal enabledelayedexpansion

echo PROMETHEUS BUILD SYSTEM
echo =======================
echo.

:: Parse arguments
set LINKING_MODE=STATIC
set RUN_TESTS=true
set USE_ASAN=false

:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="--dynamic" (
    set LINKING_MODE=DYNAMIC
    shift
    goto parse_args
)
if "%~1"=="--skip-tests" (
    set RUN_TESTS=false
    shift
    goto parse_args
)
if "%~1"=="--asan" (
    set USE_ASAN=true
    shift
    goto parse_args
)
if "%~1"=="/help" goto show_help
if "%~1"=="--help" goto show_help
echo Unknown option: %~1
goto show_help

:show_help
echo Usage: %0 [--dynamic] [--skip-tests] [--asan]
echo   --dynamic      Build with dynamic linking (DLL)
echo   --skip-tests   Skip building and running tests
echo   --asan         Enable AddressSanitizer for debugging
echo   (default)      Build with static linking and run tests
exit /b 1

:end_parse

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

:: AddressSanitizer flags
if "%USE_ASAN%"=="true" (
    set CMAKE_SANITIZER_FLAGS=-DCMAKE_CXX_FLAGS=/fsanitize=address -DCMAKE_EXE_LINKER_FLAGS=/fsanitize=address
    echo AddressSanitizer: Enabled (debugging mode)
    echo Warning: Performance will be significantly reduced
) else (
    set CMAKE_SANITIZER_FLAGS=
    echo AddressSanitizer: Disabled
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
    %CMAKE_SANITIZER_FLAGS% ^
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
echo.

:: Test building and running
if "%RUN_TESTS%"=="true" (
    echo Building prometheus tests...
    ninja prometheus_tests
    if errorlevel 1 (
        echo ERROR: Test build failed.
        exit /b 1
    )
    echo Test build completed successfully.
    echo.

    echo Running prometheus tests...
    tests\prometheus_tests.exe
    if errorlevel 1 (
        echo ERROR: Tests failed. Aborting build.
        exit /b 1
    )
    echo All tests passed successfully.
    echo.
) else (
    echo Skipping tests as requested.
    echo.
)

echo Launching prometheus_client...
client\prometheus_client.exe
cd ..

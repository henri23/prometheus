@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Prometheus Windows Build Script
echo ========================================

:: Check if Visual Studio is installed
where /q cl.exe
if %errorlevel% neq 0 (
    echo Visual Studio C++ compiler not found in PATH
    echo Attempting to locate and setup Visual Studio environment...
    
    :: Common Visual Studio installation paths
    set "vs_paths="
    set "vs_paths=!vs_paths!"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" "
    set "vs_paths=!vs_paths!"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" "
    set "vs_paths=!vs_paths!"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" "
    set "vs_paths=!vs_paths!"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" "
    set "vs_paths=!vs_paths!"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" "
    set "vs_paths=!vs_paths!"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" "
    
    set "vs_found=0"
    for %%i in (!vs_paths!) do (
        if exist %%i (
            echo Found Visual Studio at: %%i
            call %%i x64
            set "vs_found=1"
            goto :vs_setup_done
        )
    )
    
    :vs_setup_done
    if !vs_found! equ 0 (
        echo ERROR: Could not find Visual Studio installation.
        echo Please install Visual Studio 2019 or 2022 with C++ support.
        echo Or manually run vcvarsall.bat before running this script.
        pause
        exit /b 1
    )
    
    :: Verify cl.exe is now available
    where /q cl.exe
    if %errorlevel% neq 0 (
        echo ERROR: Failed to setup Visual Studio environment
        pause
        exit /b 1
    )
    
    echo Visual Studio environment setup successfully
) else (
    echo Visual Studio C++ compiler found in PATH
)

:: Check for required tools
echo Checking for required tools...

where /q cmake
if %errorlevel% neq 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake and add it to your PATH
    pause
    exit /b 1
)
echo - CMake found

where /q ninja
if %errorlevel% neq 0 (
    echo ERROR: Ninja not found in PATH
    echo Please install Ninja build system and add it to your PATH
    echo You can download it from: https://github.com/ninja-build/ninja/releases
    pause
    exit /b 1
)
echo - Ninja found

:: Check for Vulkan SDK
if not defined VULKAN_SDK (
    echo ERROR: VULKAN_SDK environment variable not set
    echo Please install the Vulkan SDK from: https://vulkan.lunarg.com/
    pause
    exit /b 1
)
echo - Vulkan SDK found at: %VULKAN_SDK%

:: Build configuration
set "BUILD_TYPE=Debug"
if "%1"=="Release" (
    set "BUILD_TYPE=Release"
)
echo Build type: %BUILD_TYPE%

:: Create build directory
set "BUILD_DIR=build_windows_%BUILD_TYPE%"
if exist "%BUILD_DIR%" (
    echo Cleaning existing build directory...
    rmdir /s /q "%BUILD_DIR%"
)
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo ========================================
echo Configuring with CMake...
echo ========================================

:: Configure with CMake using Ninja and MSVC
cmake -G "Ninja" ^
      -DCMAKE_C_COMPILER=cl.exe ^
      -DCMAKE_CXX_COMPILER=cl.exe ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
      ..

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo ========================================
echo Building with Ninja...
echo ========================================

ninja

if %errorlevel% neq 0 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

echo ========================================
echo Build completed successfully!
echo ========================================
echo Executable location: %BUILD_DIR%\client\prometheus_client.exe
echo To run the application, execute:
echo   cd %BUILD_DIR%\client
echo   prometheus_client.exe

cd ..

if "%2"=="run" (
    echo ========================================
    echo Running the application...
    echo ========================================
    cd "%BUILD_DIR%\client"
    prometheus_client.exe
    cd ..\..
)

pause
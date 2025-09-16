@echo off
setlocal enableextensions enabledelayedexpansion

echo ========================================
echo Prometheus Windows Build Script
echo ========================================

REM -------------------------------
REM Parse arguments
REM   %1 = Debug|Release (default Debug)
REM -------------------------------
set "BUILD_TYPE=Debug"
if /I "%~1"=="Release" set "BUILD_TYPE=Release"

REM -------------------------------
REM Ensure Visual C++ (cl.exe) is available (load vcvars if needed)
REM -------------------------------
where /q cl.exe
if errorlevel 1 (
    echo Visual Studio C++ compiler not found in PATH
    echo Attempting to locate and setup Visual Studio environment...

    set "VSCMD_BAT="
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -prerelease -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            if exist "%%~I\VC\Auxiliary\Build\vcvarsall.bat" (
                set "VSCMD_BAT=%%~I\VC\Auxiliary\Build\vcvarsall.bat"
                goto :have_vs
            )
        )
    )

    REM Fallback common locations
    for %%P in (
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
    ) do (
        if exist %%~P (
            set "VSCMD_BAT=%%~P"
            goto :have_vs
        )
    )

    :have_vs
    if not defined VSCMD_BAT (
        echo ERROR: Could not find a supported Visual Studio installation with C++ tools.
        pause
        exit /b 1
    )

    echo Found Visual Studio environment: "%VSCMD_BAT%"
    call "%VSCMD_BAT%" x64
    if errorlevel 1 (
        echo ERROR: Failed to initialize the Visual Studio environment.
        pause
        exit /b 1
    )

    where /q cl.exe
    if errorlevel 1 (
        echo ERROR: cl.exe is still not available after vcvarsall. Environment may be corrupted.
        pause
        exit /b 1
    )
    echo Visual Studio environment setup successfully.
) else (
    echo Visual Studio C++ compiler found in PATH
)

REM -------------------------------
REM Check required tools
REM -------------------------------
echo Checking for required tools...

where /q cmake
if errorlevel 1 (
    echo ERROR: CMake not found in PATH
    pause
    exit /b 1
)
echo - CMake found

where /q ninja
if errorlevel 1 (
    echo ERROR: Ninja not found in PATH
    pause
    exit /b 1
)
echo - Ninja found

REM -------------------------------
REM Check Vulkan SDK
REM -------------------------------
if not defined VULKAN_SDK (
    echo ERROR: VULKAN_SDK environment variable not set
    pause
    exit /b 1
)
if not exist "%VULKAN_SDK%" (
    echo ERROR: VULKAN_SDK points to a path that does not exist:
    echo        "%VULKAN_SDK%"
    pause
    exit /b 1
)
echo - Vulkan SDK found at: %VULKAN_SDK%

echo Build type: %BUILD_TYPE%

REM -------------------------------
REM Setup build directory: bin/Debug or bin/Release
REM -------------------------------
set "BUILD_DIR=bin\%BUILD_TYPE%"
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

pushd "%BUILD_DIR%" || (
    echo ERROR: Could not enter build directory "%BUILD_DIR%".
    pause
    exit /b 1
)

echo ========================================
echo Configuring with CMake...
echo ========================================

cmake -G "Ninja" ^
  -DCMAKE_C_COMPILER=cl.exe ^
  -DCMAKE_CXX_COMPILER=cl.exe ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
  ../..

if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    popd
    pause
    exit /b 1
)

echo ========================================
echo Building with Ninja...
echo ========================================

ninja
if errorlevel 1 (
    echo ERROR: Build failed.
    popd
    pause
    exit /b 1
)

echo ========================================
echo Build completed successfully!
echo ========================================

REM -------------------------------
REM Copy DLLs into client folder
REM -------------------------------
set "CLIENT_DIR=%BUILD_DIR%\client\"

if exist "%CLIENT_DIR%\prometheus_client.exe" (
    echo Copying runtime DLLs into client folder...

    REM Copy prometheus_core.dll (from core build dir)
    if exist "%BUILD_DIR%\core\prometheus_core.dll" (
        copy /Y "%BUILD_DIR%\core\prometheus_core.dll" "%CLIENT_DIR%" >nul
        echo - Copied prometheus_core.dll
    ) else (
        echo WARNING: prometheus_core.dll not found
    )

    REM Copy SDL3.dll (from SDL3 build dir or root)
    if exist "%BUILD_DIR%\SDL3.dll" (
        copy /Y "%BUILD_DIR%\SDL3.dll" "%CLIENT_DIR%" >nul
        echo - Copied SDL3.dll
    ) else if exist "%BUILD_DIR%\external\SDL3\SDL3.dll" (
        copy /Y "%BUILD_DIR%\external\SDL3\SDL3.dll" "%CLIENT_DIR%" >nul
        echo - Copied SDL3.dll
    ) else (
        echo WARNING: SDL3.dll not found
    )
)

REM -------------------------------
REM Always run application
REM -------------------------------
if exist "%CLIENT_DIR%\prometheus_client.exe" (
    echo ========================================
    echo Running the application...
    echo ========================================
    pushd "%CLIENT_DIR%"
    start /wait "" prometheus_client.exe
    set "RUN_ERR=%ERRORLEVEL%"
    popd
    if not "%RUN_ERR%"=="0" (
        echo Application exited with code %RUN_ERR%.
    )
) else (
    echo ERROR: prometheus_client.exe not found, cannot run.
)

popd

REM Keep window open if launched by Explorer
if /I "%CMDCMDLINE%"=="%CMDCMDLINE:\=%" (
    REM already in console
) else (
    pause
)

endlocal


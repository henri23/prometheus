@echo off
setlocal EnableDelayedExpansion

:: Modern Build System for Prometheus - Windows Version
:: Features: ASCII art, progress tracking, colored output, tool checking

:: Enable ANSI color support for Windows 10+
for /f "tokens=2 delims=." %%i in ('ver') do set winver=%%i
if %winver% geq 10 (
    :: Enable virtual terminal processing for color support
    reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f >nul 2>&1
)

:: ANSI Color codes (works on Windows 10+ and Windows Terminal)
set "RED=[31m"
set "GREEN=[32m"
set "YELLOW=[33m"
set "BLUE=[34m"
set "MAGENTA=[35m"
set "CYAN=[36m"
set "WHITE=[37m"
set "BOLD=[1m"
set "DIM=[2m"
set "RESET=[0m"

:: Unicode symbols (fallback to ASCII if needed)
set "CHECK=√"
set "CROSS=×"
set "PROGRESS_FULL=█"
set "PROGRESS_EMPTY=░"

:: Global variables
set "CURRENT_PROGRESS=0"
set "LOG_COUNT=0"

:: Create temporary log file
set "LOGFILE=%TEMP%\prometheus_build.log"
echo. > "%LOGFILE%"

:: Cleanup function
set "CLEANUP_DONE=false"

:cleanup
if "%CLEANUP_DONE%"=="true" exit /b
set "CLEANUP_DONE=true"
if exist "%LOGFILE%" del "%LOGFILE%" >nul 2>&1
exit /b

:: Set cleanup trap
if not defined PROMETHEUS_BUILD_RUNNING (
    set "PROMETHEUS_BUILD_RUNNING=1"
    call :main
    call :cleanup
    exit /b
)

:show_header
echo %CYAN%%BOLD%
echo     ██████╗ ██████╗  ██████╗ ███╗   ███╗███████╗████████╗██╗  ██╗███████╗██╗   ██╗███████╗
echo     ██╔══██╗██╔══██╗██╔═══██╗████╗ ████║██╔════╝╚══██╔══╝██║  ██║██╔════╝██║   ██║██╔════╝
echo     ██████╔╝██████╔╝██║   ██║██╔████╔██║█████╗     ██║   ███████║█████╗  ██║   ██║███████╗
echo     ██╔═══╝ ██╔══██╗██║   ██║██║╚██╔╝██║██╔══╝     ██║   ██╔══██║██╔══╝  ██║   ██║╚════██║
echo     ██║     ██║  ██║╚██████╔╝██║ ╚═╝ ██║███████╗   ██║   ██║  ██║███████╗╚██████╔╝███████║
echo     ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚══════╝
echo %RESET%%DIM%                              Modern Build System for Windows v1.0%RESET%
echo.
goto :eof

:show_progress
set /a "filled=!CURRENT_PROGRESS!/2"
set /a "empty=50-!filled!"

set "bar="
for /l %%i in (1,1,!filled!) do set "bar=!bar!%PROGRESS_FULL%"
for /l %%i in (1,1,!empty!) do set "bar=!bar!%PROGRESS_EMPTY%"

echo %CYAN%Progress: [%GREEN%!bar!%RESET%%CYAN%] %WHITE%!CURRENT_PROGRESS!%%%RESET%
echo.
goto :eof

:show_steps
echo %WHITE%%BOLD%Build Steps:%RESET%
if !CURRENT_PROGRESS! geq 20 (
    echo   %GREEN%!CHECK! Tool Check%RESET%
) else if !CURRENT_PROGRESS! geq 5 (
    echo   %YELLOW%⚡ Tool Check%RESET% %DIM%^(in progress^)%RESET%
) else (
    echo   %DIM%○ Tool Check%RESET%
)

if !CURRENT_PROGRESS! geq 40 (
    echo   %GREEN%!CHECK! CMake Configuration%RESET%
) else if !CURRENT_PROGRESS! geq 25 (
    echo   %YELLOW%⚡ CMake Configuration%RESET% %DIM%^(in progress^)%RESET%
) else (
    echo   %DIM%○ CMake Configuration%RESET%
)

if !CURRENT_PROGRESS! geq 60 (
    echo   %GREEN%!CHECK! Build Setup%RESET%
) else if !CURRENT_PROGRESS! geq 45 (
    echo   %YELLOW%⚡ Build Setup%RESET% %DIM%^(in progress^)%RESET%
) else (
    echo   %DIM%○ Build Setup%RESET%
)

if !CURRENT_PROGRESS! geq 80 (
    echo   %GREEN%!CHECK! Compilation%RESET%
) else if !CURRENT_PROGRESS! geq 65 (
    echo   %YELLOW%⚡ Compilation%RESET% %DIM%^(in progress^)%RESET%
) else (
    echo   %DIM%○ Compilation%RESET%
)

if !CURRENT_PROGRESS! geq 100 (
    echo   %GREEN%!CHECK! Finalization%RESET%
) else if !CURRENT_PROGRESS! geq 85 (
    echo   %YELLOW%⚡ Finalization%RESET% %DIM%^(in progress^)%RESET%
) else (
    echo   %DIM%○ Finalization%RESET%
)
echo.
goto :eof

:show_logs
echo %WHITE%%BOLD%Build Log:%RESET%
echo %DIM%────────────────────────────────────────────────────────%RESET%

:: Show last 8 log entries
set "log_start=1"
if !LOG_COUNT! gtr 8 set /a "log_start=!LOG_COUNT!-7"

for /l %%i in (!log_start!,1,!LOG_COUNT!) do (
    if exist "%LOGFILE%" (
        for /f "skip=%%i tokens=*" %%a in ('type "%LOGFILE%" 2^>nul ^| findstr /n "^"') do (
            set "line=%%a"
            set "line=!line:*:=!"
            if not "!line!"=="" echo %DIM%!line!%RESET%
            goto :next_log
        )
        :next_log
    )
)

:: Fill remaining lines to maintain consistent height
set /a "lines_shown=!LOG_COUNT!"
if !lines_shown! gtr 8 set "lines_shown=8"
set /a "empty_lines=8-!lines_shown!"
for /l %%i in (1,1,!empty_lines!) do echo.

echo %DIM%────────────────────────────────────────────────────────%RESET%
goto :eof

:add_log
set "timestamp="
for /f "tokens=1-3 delims=:." %%a in ("%time%") do (
    set "timestamp=%%a:%%b:%%c"
)
set "timestamp=!timestamp: =0!"

set /a "LOG_COUNT+=1"
echo [!timestamp!] %~1 >> "%LOGFILE%"
goto :eof

:update_display
cls
call :show_header
call :show_progress
call :show_steps
call :show_logs
goto :eof

:animate_progress
set "target_progress=%~1"
:animate_loop
if !CURRENT_PROGRESS! geq !target_progress! goto :eof
set /a "CURRENT_PROGRESS+=1"
call :update_display
timeout /t 0 >nul 2>&1
ping -n 1 127.0.0.1 >nul
goto :animate_loop

:check_tool
set "tool=%~1"
set "hint=%~2"

where %tool% >nul 2>&1
if !errorlevel! equ 0 (
    call :add_log "√ Found %tool%"
    exit /b 0
) else (
    call :add_log "× Missing %tool%"
    if not "%hint%"=="" call :add_log "  Install: %hint%"
    exit /b 1
)

:run_command
set "cmd=%~1"
set "description=%~2"

call :add_log "Starting: %description%"
call :update_display

set "temp_output=%TEMP%\prometheus_cmd_output.txt"

%cmd% > "%temp_output%" 2>&1
set "cmd_result=!errorlevel!"

if !cmd_result! equ 0 (
    call :add_log "√ Completed: %description%"
    if exist "%temp_output%" del "%temp_output%" >nul 2>&1
    exit /b 0
) else (
    call :add_log "× Failed: %description%"
    if exist "%temp_output%" (
        for /f "tokens=*" %%a in ('type "%temp_output%" 2^>nul') do (
            call :add_log "  ERROR: %%a"
        )
        del "%temp_output%" >nul 2>&1
    )
    exit /b 1
)

:main
:: Record start time (Windows doesn't have milliseconds easily, so we'll use seconds)
for /f "tokens=1-4 delims=:.," %%a in ("%time%") do (
    set /a "start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

:: Initialize display
call :update_display

:: Step 1: Tool Check (0-20%)
call :add_log "Checking required build tools..."
call :animate_progress 5

set "tools_ok=true"
call :check_tool "cmake" "Install Visual Studio or CMake tools"
if !errorlevel! neq 0 set "tools_ok=false"

call :check_tool "ninja" "Install ninja-build via chocolatey or vcpkg"
if !errorlevel! neq 0 set "tools_ok=false"

call :check_tool "cl" "Install Visual Studio with C++ tools"
if !errorlevel! neq 0 (
    call :check_tool "clang++" "Install LLVM/Clang"
    if !errorlevel! neq 0 set "tools_ok=false"
)

call :check_tool "git" "Install Git for Windows"
if !errorlevel! neq 0 set "tools_ok=false"

if "!tools_ok!"=="false" (
    call :add_log "Some required tools are missing. Please install them."
    call :update_display
    pause
    exit /b 1
)

call :add_log "All required tools are available!"
call :animate_progress 20
timeout /t 1 >nul

:: Step 2: CMake Configuration (20-40%)
call :add_log "Creating build directory..."
call :animate_progress 25

if not exist "bin" mkdir bin

:: Set compiler preference (try Visual Studio first, then Clang)
set "cmake_compiler="
where cl >nul 2>&1
if !errorlevel! equ 0 (
    set "cmake_compiler=-DCMAKE_CXX_COMPILER=cl"
) else (
    where clang++ >nul 2>&1
    if !errorlevel! equ 0 set "cmake_compiler=-DCMAKE_CXX_COMPILER=clang++"
)

call :run_command "cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=YES !cmake_compiler! -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POSITION_INDEPENDENT_CODE=ON -B bin ." "CMake configuration"
if !errorlevel! neq 0 (
    call :add_log "CMake configuration failed"
    call :update_display
    pause
    exit /b 1
)

call :animate_progress 40
timeout /t 1 >nul

:: Step 3: Build Setup (40-60%)
call :add_log "Setting up IDE integration..."
call :animate_progress 45

cd bin
if exist "compile_commands.json" (
    if exist "..\compile_commands.json" del "..\compile_commands.json" >nul 2>&1
    mklink "..\compile_commands.json" "bin\compile_commands.json" >nul 2>&1
    call :add_log "√ Created compile_commands.json link"
) else (
    call :add_log "Warning: compile_commands.json not found"
)

call :animate_progress 60
timeout /t 1 >nul

:: Step 4: Compilation (60-80%)
call :run_command "ninja prometheus_client" "Building prometheus client"
if !errorlevel! neq 0 (
    call :add_log "Build failed"
    call :update_display
    pause
    exit /b 1
)

call :animate_progress 80
timeout /t 1 >nul

:: Step 5: Finalization (80-100%)
call :animate_progress 85

:: Calculate build time (approximate, since Windows batch is limited)
for /f "tokens=1-4 delims=:.," %%a in ("%time%") do (
    set /a "end_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /a "build_time=!end_time!-!start_time!"
if !build_time! lss 0 set /a "build_time+=8640000"
set /a "build_seconds=!build_time!/100"

call :add_log ""
call :add_log "🎉 BUILD COMPLETED SUCCESSFULLY! 🎉"
call :add_log "Build time: ~!build_seconds! seconds"
call :add_log "Executable: .\bin\client\prometheus_client.exe"
call :add_log ""
call :add_log "Press any key to launch or Ctrl+C to exit..."

call :animate_progress 100

:: Final display with logs visible
call :update_display
echo.
echo %YELLOW%%BOLD%Press any key to launch the application, or Ctrl+C to exit...%RESET%
pause >nul

echo %CYAN%%BOLD%Launching Prometheus Client...%RESET%
echo.
.\client\prometheus_client.exe

goto :eof
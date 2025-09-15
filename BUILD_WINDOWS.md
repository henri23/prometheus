# Windows Build Instructions

This document explains how to build Prometheus on Windows using the provided build scripts.

## Prerequisites

1. **Visual Studio 2019 or 2022** with C++ support
   - Community, Professional, or Enterprise edition
   - Make sure "C++ CMake tools for Visual Studio" is installed

2. **CMake** (3.16 or later)
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

3. **Ninja Build System**
   - Download from: https://github.com/ninja-build/ninja/releases
   - Extract `ninja.exe` to a folder in your PATH

4. **Vulkan SDK**
   - Download from: https://vulkan.lunarg.com/
   - The installer should set the `VULKAN_SDK` environment variable

## Build Scripts

Two build scripts are provided:

### Option 1: Batch Script (Recommended)
```cmd
build_windows.bat [Release] [run]
```

- Automatically locates and sets up Visual Studio environment
- Builds in Debug mode by default, pass "Release" for Release build
- Pass "run" as second parameter to automatically run after building

Examples:
```cmd
# Debug build
build_windows.bat

# Release build
build_windows.bat Release

# Debug build and run
build_windows.bat Debug run

# Release build and run
build_windows.bat Release run
```

### Option 2: PowerShell Script
```powershell
.\build_windows.ps1 [-BuildType Debug|Release] [-Run]
```

**Note:** The PowerShell script requires you to manually run `vcvarsall.bat` first to set up the Visual Studio environment.

Examples:
```powershell
# Debug build
.\build_windows.ps1

# Release build
.\build_windows.ps1 -BuildType Release

# Debug build and run
.\build_windows.ps1 -Run

# Release build and run
.\build_windows.ps1 -BuildType Release -Run
```

## Manual Setup (if scripts fail)

1. Open "Developer Command Prompt for VS 2022" (or VS 2019)
   - This automatically sets up the Visual Studio environment

2. Navigate to the project directory:
   ```cmd
   cd path\to\prometheus
   ```

3. Create and enter build directory:
   ```cmd
   mkdir build_windows
   cd build_windows
   ```

4. Configure with CMake:
   ```cmd
   cmake -G "Ninja" -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe -DCMAKE_BUILD_TYPE=Debug ..
   ```

5. Build with Ninja:
   ```cmd
   ninja
   ```

6. Run the executable:
   ```cmd
   cd client
   prometheus_client.exe
   ```

## Troubleshooting

### Visual Studio Not Found
- Ensure Visual Studio 2019 or 2022 is installed with C++ support
- Try running from "Developer Command Prompt for VS"

### CMake Not Found
- Install CMake and ensure it's added to PATH
- Restart command prompt after installation

### Ninja Not Found
- Download ninja.exe and place it in a directory that's in your PATH
- Or extract to project directory and use `.\ninja.exe`

### Vulkan SDK Not Found
- Install Vulkan SDK from LunarG
- Ensure `VULKAN_SDK` environment variable is set
- Restart command prompt after installation

### Build Errors
- Ensure all dependencies are properly installed
- Check that you're using 64-bit tools (x64)
- Try cleaning build directory and rebuilding

## Windows-Specific Features

The codebase includes Windows-specific optimizations:
- MSVC compiler optimizations
- Windows-specific debug break functionality
- Proper DLL export/import macros for `__declspec(dllexport)`

The CMake configuration automatically:
- Sets high warning levels (`/W4`)
- Disables unnecessary security warnings
- Prevents Windows header macro conflicts (`NOMINMAX`)
- Reduces Windows header bloat (`WIN32_LEAN_AND_MEAN`)
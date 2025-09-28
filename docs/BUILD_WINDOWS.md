# Windows Build Guide

This guide explains how to build Prometheus Engine on Windows using the provided build scripts and tools.

## Prerequisites

### System Requirements

- **Operating System**: Windows 10 (version 1903+) or Windows 11
- **Compiler**: Visual Studio 2019 or 2022 with C++ support
- **CMake**: 3.16 or later
- **Git**: With submodule support
- **Graphics**: Vulkan-compatible GPU with updated drivers

### Required Software

1. **Visual Studio 2019 or 2022** with C++ support
   - Community, Professional, or Enterprise edition
   - Install "Desktop development with C++" workload
   - Ensure "C++ CMake tools for Visual Studio" is included

2. **CMake** (3.16 or later)
   - Download from: https://cmake.org/download/
   - **Important**: Add to PATH during installation
   - Verify: Open Command Prompt and run `cmake --version`

3. **Ninja Build System**
   - Download from: https://github.com/ninja-build/ninja/releases
   - Extract `ninja.exe` to a folder in your PATH
   - Alternative: Install via Visual Studio installer

4. **Vulkan SDK** (1.3+ recommended)
   - Download from: https://vulkan.lunarg.com/
   - The installer should automatically set `VULKAN_SDK` environment variable
   - Verify: Run `vulkaninfo` in Command Prompt

5. **Git** with submodule support
   - Download from: https://git-scm.com/download/win
   - During installation, select "Git from the command line and also from 3rd-party software"

## Quick Build

### Option 1: Modern Build Script (Recommended)

```cmd
# Clone repository with submodules
git clone --recursive https://github.com/your-username/prometheus.git
cd prometheus

# Build with modern UI (progress tracking, colored output)
build-modern.bat

# Run the application
bin\client\prometheus_client.exe
```

### Option 2: Ninja Build Script

```cmd
# Clone repository
git clone --recursive https://github.com/your-username/prometheus.git
cd prometheus

# Build with ninja (fast and clean)
build-ninja.bat

# Run the application
bin\client\prometheus_client.exe
```

## Build Scripts Explained

### `build-modern.bat` - Advanced Build UI

Features:
- **ASCII art header** and modern terminal interface
- **Automatic Visual Studio detection** and environment setup
- **Progress tracking** with colored output
- **Error handling** and detailed diagnostics
- **Multiple build configurations** (Debug/Release)

Usage:
```cmd
build-modern.bat                    # Debug build
build-modern.bat Release            # Release build
build-modern.bat Debug run          # Debug build and run
build-modern.bat Release run        # Release build and run
```

### `build-ninja.bat` - Fast Clean Builds

Features:
- **Fast compilation** with Ninja build system
- **Automatic tool detection** and validation
- **Clean error reporting** with colored output
- **IDE integration** with compile_commands.json

Usage:
```cmd
build-ninja.bat                     # Debug build
set BUILD_TYPE=Release & build-ninja.bat  # Release build
```

## Run the executable
   ```cmd
   cd client
   prometheus_client.exe
   ```

## Verification and Testing

### Verify Installation

```cmd
# Test CMake
cmake --version

# Test Ninja
ninja --version

# Test Vulkan
vulkaninfo --summary

# Test Visual Studio compiler
cl.exe
```

### Test the Build

```cmd
# Quick smoke test
bin\client\prometheus_client.exe --version

# Run with validation layers (debug builds)
set VK_LAYER_PATH=%VULKAN_SDK%\Bin
bin\client\prometheus_client.exe
```

## Troubleshooting

### Installation Issues

**Problem:** Visual Studio not found
```cmd
# Solution: Verify installation
"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
# Install "Desktop development with C++" workload
```

**Problem:** CMake not found
```cmd
# Solution: Add CMake to PATH
# Add: C:\Program Files\CMake\bin
# Restart Command Prompt
cmake --version
```

**Problem:** Ninja not found
```cmd
# Solution: Download and extract ninja.exe
# Place in: C:\tools\ninja\ninja.exe
# Add C:\tools\ninja to PATH
```

**Problem:** Vulkan SDK not found
```cmd
# Solution: Check environment variable
echo %VULKAN_SDK%
# Should show: C:\VulkanSDK\1.3.xxx.x
# If empty, reinstall Vulkan SDK
```

### Build Issues

**Problem:** `MSVC not found` or `cl.exe not recognized`
```cmd
# Solution: Use Developer Command Prompt
# Or run vcvarsall.bat manually:
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

**Problem:** `error C2039: 'min': is not a member of 'std'`
```cmd
# Solution: Windows.h macro conflict
# The code already defines NOMINMAX, check for conflicting headers
```

**Problem:** `LNK2019: unresolved external symbol`
```cmd
# Solution: Missing library dependencies
# Ensure Vulkan SDK is properly installed
# Check that VULKAN_SDK environment variable is set
```

### Runtime Issues

**Problem:** `prometheus_core.dll not found`
```cmd
# Solution: Library path issue
# Run from bin directory:
cd bin
client\prometheus_client.exe
```

**Problem:** Vulkan device creation failed
```cmd
# Solution: Update graphics drivers
# NVIDIA: Download latest drivers from nvidia.com
# AMD: Download latest drivers from amd.com
# Intel: Update through Windows Update or intel.com
```

## Advanced Configuration

### Environment Variables

```cmd
# Enable debug output
set PROMETHEUS_DEBUG=1

# Vulkan debugging
set VK_LAYER_PATH=%VULKAN_SDK%\Bin

# Custom build configuration
set BUILD_TYPE=Release
set CMAKE_GENERATOR=Ninja
```

### IDE Integration

**Visual Studio:**
- Open project folder in Visual Studio
- VS automatically detects CMake configuration
- Use "Select Startup Item" to choose prometheus_client

**VS Code:**
```json
// .vscode/c_cpp_properties.json
{
    "configurations": [{
        "name": "Win32",
        "compileCommands": "${workspaceFolder}/compile_commands.json",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "msvc-x64"
    }]
}
```

### Custom Build Targets

```cmd
# Build only core library
ninja prometheus_core

# Build only client
ninja prometheus_client

# Clean build
ninja clean

# Verbose output
ninja -v
```

## Performance Notes

**Typical build times (Release mode):**
- **4-core system**: ~5-8 minutes (full build)
- **8-core system**: ~2-4 minutes (full build)
- **Incremental**: ~15-45 seconds

**Memory usage:**
- **Peak RAM**: ~3-6 GB during compilation
- **Disk space**: ~800 MB for full build

**Windows-Specific Optimizations:**

The codebase includes Windows-specific features:
- **MSVC compiler optimizations** - High warning levels (`/W4`) and performance flags
- **Windows debug support** - Integrated debug break functionality
- **Proper DLL handling** - Export/import macros with `__declspec(dllexport)`
- **Header optimization** - `NOMINMAX` and `WIN32_LEAN_AND_MEAN` to reduce compile times

---

For Linux builds, see [BUILD_LINUX.md](BUILD_LINUX.md)
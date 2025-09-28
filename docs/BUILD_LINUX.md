# Linux Build Guide

This guide explains how to build Prometheus Engine on Linux using the provided build scripts and tools.

## Prerequisites

### System Requirements

- **Operating System**: Ubuntu 20.04+, Arch Linux, Fedora 35+, or compatible distribution
- **Compiler**: GCC 8+ or Clang 7+ with C++17 support
- **CMake**: 3.16 or later
- **Git**: With submodule support
- **Graphics**: Vulkan-compatible GPU with updated drivers

### Required Packages

**Ubuntu/Debian:**
```bash
# Essential build tools
sudo apt update
sudo apt install build-essential cmake ninja-build git

# Graphics and development libraries
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
sudo apt install libgl1-mesa-dev libglu1-mesa-dev

# Audio libraries (for SDL3)
sudo apt install libasound2-dev libpulse-dev libjack-jackd2-dev
```

**Arch Linux:**
```bash
# Essential build tools
sudo pacman -S base-devel cmake ninja git

# Graphics and development libraries
sudo pacman -S vulkan-devel vulkan-validation-layers vulkan-tools spirv-tools
sudo pacman -S libx11 libxrandr libxinerama libxcursor libxi
sudo pacman -S mesa

# Audio libraries
sudo pacman -S alsa-lib libpulse jack2
```

**Fedora:**
```bash
# Essential build tools
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake ninja-build git

# Graphics and development libraries
sudo dnf install vulkan-devel vulkan-validation-layers vulkan-tools spirv-tools
sudo dnf install libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# Audio libraries
sudo dnf install alsa-lib-devel pulseaudio-libs-devel jack-audio-connection-kit-devel
```

### Graphics Drivers

**NVIDIA:**
```bash
# Ubuntu/Debian
sudo apt install nvidia-driver-535 nvidia-utils-535
# Arch Linux
sudo pacman -S nvidia nvidia-utils
# Fedora
sudo dnf install akmod-nvidia xorg-x11-drv-nvidia-cuda
```

**AMD:**
```bash
# Ubuntu/Debian
sudo apt install mesa-vulkan-drivers
# Arch Linux
sudo pacman -S vulkan-radeon
# Fedora
sudo dnf install mesa-vulkan-drivers
```

**Intel:**
```bash
# Ubuntu/Debian
sudo apt install mesa-vulkan-drivers intel-media-va-driver
# Arch Linux
sudo pacman -S vulkan-intel
# Fedora
sudo dnf install mesa-vulkan-drivers
```

##  Quick Build

### Option 1: Ninja Build

```bash
# Clone repository with submodules
git clone --recursive https://github.com/your-username/prometheus.git
cd prometheus

# Build with ninja (fast and clean output)
./build-ninja.sh

# Run the application
./bin/client/prometheus_client
```

### Option 2: Modern Build Script (Same as option 1 but more beautiful)

```bash
# Clone repository
git clone --recursive https://github.com/your-username/prometheus.git
cd prometheus

# Build with advanced UI (progress tracking, logs)
./build-modern.sh

# Run the application
./bin/client/prometheus_client
```

## Build Scripts Explained

### `build-ninja.sh` - Fast Clean Builds

Features:
- **Fast compilation** with Ninja build system
- **Colored output** with Unicode symbols
- **Clean error reporting** and status messages
- **Automatic tool detection** and validation
- **IDE integration** with compile_commands.json generation

Usage:
```bash
./build-ninja.sh           # Debug build
BUILD_TYPE=Release ./build-ninja.sh  # Release build
```

### `build-modern.sh` - Advanced Build UI

Features:
- **ASCII art header** and modern terminal UI
- **Real-time progress tracking** with smooth animations
- **Scrolling build logs** with syntax highlighting
- **Memory usage tracking** and build statistics
- **Error highlighting** and detailed diagnostics

Usage:
```bash
./build-modern.sh          # Interactive build with UI
./build-modern.sh --quiet  # Minimal output mode
```

## Run the Application

```bash
cd client
./prometheus_client
```

##  Verification and Testing

### Verify Vulkan Installation

```bash
# Check Vulkan loader
vulkaninfo --summary

# Verify validation layers
vulkaninfo | grep -i validation

# Test Vulkan functionality
vkcube  # If available
```

### Test the Build

```bash
# Quick smoke test
./bin/client/prometheus_client --version

# Run with validation layers (debug builds)
VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d \
./bin/client/prometheus_client

# Check for memory leaks (if valgrind is installed)
valgrind --tool=memcheck --leak-check=full \
./bin/client/prometheus_client
```

##  Troubleshooting

### Vulkan Issues

**Problem:** `vkCreateInstance failed`
```bash
# Solution: Install Vulkan drivers and SDK
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev

# Verify installation
vulkaninfo --summary
```

**Problem:** Validation layers not found
```bash
# Solution: Set validation layer path
export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d
```

### Build Issues

**Problem:** `CMake not found`
```bash
# Solution: Install CMake
sudo apt install cmake
# or use snap for latest version
sudo snap install cmake --classic
```

**Problem:** `Ninja not found`
```bash
# Solution: Install Ninja
sudo apt install ninja-build
# Create symlink if needed
sudo ln -s /usr/bin/ninja-build /usr/bin/ninja
```

**Problem:** `SDL3 build errors`
```bash
# Solution: Install development libraries
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
sudo apt install libasound2-dev libpulse-dev
```

**Problem:** `Compiler not found or too old`
```bash
# Check compiler version
gcc --version
g++ --version

# Install newer GCC (Ubuntu)
sudo apt install gcc-10 g++-10
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100
```

### Runtime Issues

**Problem:** `libprometheus_core.so not found`
```bash
# Solution: Set library path
export LD_LIBRARY_PATH="$(pwd)/bin/core:$LD_LIBRARY_PATH"
# or run from bin directory
cd bin && ./client/prometheus_client
```

**Problem:** Graphics drivers not working
```bash
# Check driver status
lspci -v | grep -A12 VGA
nvidia-smi  # For NVIDIA
clinfo      # For OpenCL/compute

# Reinstall mesa drivers
sudo apt install --reinstall mesa-vulkan-drivers
```

### Performance Issues

**Problem:** Low FPS or stuttering
```bash
# Run with performance monitoring
perf stat ./bin/client/prometheus_client

# Check GPU usage
nvidia-smi -l 1  # NVIDIA
radeontop        # AMD
```

## Advanced Configuration

### Environment Variables

```bash
# Enable debug output
export PROMETHEUS_DEBUG=1

# Vulkan debugging
export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d

# Force specific GPU (multi-GPU systems)
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.json

# Build configuration
export CC=clang
export CXX=clang++
export BUILD_TYPE=Release
```

### IDE Integration

**VS Code:**
```json
// .vscode/c_cpp_properties.json
{
    "configurations": [{
        "name": "Linux",
        "compileCommands": "${workspaceFolder}/compile_commands.json",
        "cStandard": "c17",
        "cppStandard": "c++17"
    }]
}
```

**CLion:**
- Open project folder
- CLion automatically detects CMake configuration
- Use "Debug" or "Release" build configurations

### Custom Build Targets

```bash
# Build only core library
ninja prometheus_core

# Build only client
ninja prometheus_client

# Clean build
ninja clean

# Verbose output
ninja -v
```

## Build Performance

**Typical build times (Release mode):**
- **4-core system**: ~3-5 minutes (full build)
- **8-core system**: ~1-2 minutes (full build)
- **Incremental**: ~10-30 seconds

**Memory usage:**
- **Peak RAM**: ~2-4 GB during compilation
- **Disk space**: ~500 MB for full build

---

For Windows builds, see [BUILD_WINDOWS.md](BUILD_WINDOWS.md)
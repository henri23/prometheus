# Prometheus

A modern, cross-platform game engine built with **Vulkan**, **ImGui**, and **SDL3**. Designed with a clean core-client architecture for performance, maintainability, and extensibility.

## Features

- **Modern Vulkan Renderer** - High-performance graphics with validation layers and debugging support
- **Component-Based UI System** - Extensible ImGui interface with dockspace, custom titlebar, and font management
- **Memory Management** - Tagged allocation system with arena allocators for optimal performance
- **Cross-Platform** - Native support for Windows (MSVC) and Linux (GCC/Clang)
- **Asset Pipeline** - Efficient loading for images, fonts, and embedded resources
- **Developer Tools** - Modern build scripts, IDE integration, and comprehensive logging

## Quick Start

### Prerequisites

- **C++17** compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- **CMake** 3.16 or later
- **Vulkan SDK** 1.3+ with validation layers
- **Git** with submodule support

### Build

**Linux:**
```bash
git clone --recursive https://github.com/your-username/prometheus.git
cd prometheus
./build-ninja.sh
```

**Windows:**
```cmd
git clone --recursive https://github.com/your-username/prometheus.git
cd prometheus
build-modern.bat
```

For detailed platform-specific instructions, see:
- [Linux Build Guide](build_linux.md)
- [Windows Build Guide](build_windows.md)

## Project Structure

```
prometheus/
├── core/                  # Engine core library (~6.5k LOC)
│   ├── src/assets/        # Asset loading and management
│   ├── src/core/          # Application framework and entry
│   ├── src/memory/        # Memory management (arena & pool)
│   ├── src/platform/      # Platform layer (SDL3)
│   ├── src/renderer/      # Vulkan rendering backend
│   └── src/ui/            # ImGui-based UI framework
├── client/                # Application layer (~240 LOC)
│   └── src/               # Client-specific UI and logic
├── external/              # Third-party dependencies
│   ├── SDL3/              # Platform layer and input handling
│   ├── imgui/             # Immediate mode GUI w/ docking
│   ├── spdlog/            # High-performance logging
│   └── stb_image/         # Image loading library
├── assets/                # Game assets and resources
│   ├── fonts/             # Font files (Roboto family)
│   ├── icons/             # UI icons and graphics
│   └── images/            # Textures and images
└── bin/                   # Build output directory
```

## Architecture

### Core Engine (`prometheus_core.dll/so`)

The core engine provides fundamental systems and abstractions:

- **Application Framework** - Main loop, initialization, and event handling
- **Platform Layer** - SDL3-based windowing, input, and system integration
- **Vulkan Renderer** - Modern graphics API with command buffer management
- **UI System** - Component-based ImGui framework with dockspace and custom titlebar
- **Asset Management** - Efficient resource loading with caching
- **Memory System** - Tagged allocators and arena memory management

### Client Layer (`prometheus_client`)

The client layer implements application-specific functionality:

- **UI Components** - Custom windows, menus, and interface elements
- **Application Logic** - Game or application-specific code
- **Configuration** - Client-specific settings and initialization

### Key Design Principles

- **Separation of Concerns** - Clear boundaries between engine and application code
- **Component Architecture** - Modular, extensible systems
- **Modern C++** - RAII, type safety, and performance-focused design
- **Cross-Platform** - Consistent behavior across Windows and Linux

## Dependencies

### Core Dependencies
- **[SDL3](https://github.com/libsdl-org/SDL)** - Platform abstraction and window management
- **[ImGui](https://github.com/ocornut/imgui)** - Immediate mode GUI (docking branch)
- **[spdlog](https://github.com/gabime/spdlog)** - Fast, header-only logging library
- **[stb_image](https://github.com/nothings/stb)** - Image loading and manipulation
- **Vulkan SDK** - Modern graphics API

### Build Tools
- **CMake** - Cross-platform build system
- **Ninja** - Fast, small build system (recommended)

All dependencies are managed through git submodules or system packages.

## Development

### Build Scripts

The project includes sophisticated build scripts with modern UI:

- **`build-ninja.sh`** (Linux) - Fast ninja-based builds with colored output
- **`build-modern.sh`** (Linux) - Advanced build with progress tracking and logs
- **`build-ninja.bat`** (Windows) - Windows equivalent with Visual Studio integration

### IDE Integration

- **compile_commands.json** - Generated automatically for LSP/clangd support
- **CLion/Visual Studio** - Native CMake project support
- **VS Code** - C++ extension with IntelliSense support

### Code Quality

- **Static Assertions** - Compile-time type and size validation
- **Vulkan Validation** - Debug layer integration for graphics debugging
- **Memory Tracking** - Tagged allocation system for leak detection
- **Consistent Style** - `.clang-format` configuration included

## Roadmap

See [TODO.md](TODO.md) for the detailed project roadmap, including:

- Engine core improvements and optimizations
- Advanced rendering features (PBR, shadows, post-processing)
- Additional UI components and themes
- Asset pipeline enhancements
- Platform expansion (macOS, mobile)
- Documentation and tutorials

## License

This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details.

## Contributing

TBD

### Getting Started
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

**Built with ❤️ and modern C-style C++**

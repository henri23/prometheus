# Contributing to Voltrum

Thank you for your interest in contributing to Voltrum! This document provides guidelines and information for contributors.

## Quick Start

1. **Fork** the repository on GitHub
2. **Clone** your fork locally
3. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
4. **Make** your changes
5. **Test** your changes thoroughly
6. **Commit** your changes (`git commit -m 'Add amazing feature'`)
7. **Push** to your branch (`git push origin feature/amazing-feature`)
8. **Open** a Pull Request

## Development Setup

### Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.16 or later
- Vulkan SDK 1.3+ with validation layers
- Git with submodule support

### Building

**Linux:**
```bash
git clone --recursive https://github.com/your-username/voltrum.git
cd voltrum
./build-ninja.sh
```

**Windows:**
```cmd
git clone --recursive https://github.com/your-username/voltrum.git
cd voltrum
build-ninja.bat
```

For detailed instructions, see our [build guides](docs/).

## How to Contribute

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates. When you create a bug report, include:

- **Clear description** of the issue
- **Steps to reproduce** the problem
- **Expected vs actual behavior**
- **Environment details** (OS, compiler, Vulkan driver version)
- **Log output** if available
- **Screenshots** for visual issues

### Suggesting Features

Feature suggestions are welcome! Please provide:

- **Clear description** of the feature
- **Use case** and rationale
- **Proposed implementation** (if you have ideas)
- **Impact assessment** on existing functionality

### Code Contributions

We welcome code contributions! Please ensure:

#### Code Quality
- **Follow existing code style** (see Code Style section below)
- **Write clean, readable code** with appropriate comments
- **Include tests** for new functionality
- **Ensure all tests pass** before submitting
- **Update documentation** as needed

#### Pull Request Process
1. **Update documentation** for any new features
2. **Add tests** that cover your changes
3. **Ensure CI passes** (build and tests)
4. **Request review** from maintainers
5. **Address feedback** promptly

## Code Style

Voltrum follows a C-style C++ approach with specific conventions:

### General Principles
- **Readable code over clever code**
- **Consistency with existing codebase**
- **Modern C++ features where appropriate**
- **Performance-conscious design**

### Naming Conventions
```cpp
// Types: PascalCase
struct Memory_Arena;
enum class Renderer_Type;

// Functions: snake_case
b8 initialize_renderer();
void render_frame();

// Variables: snake_case
u32 frame_count;
f32 delta_time;

// Constants: UPPER_SNAKE_CASE
constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

// Macros: UPPER_SNAKE_CASE
#define CORE_ASSERT(condition)
```

### Code Organization
- **Header files**: Use `#pragma once`
- **Includes**: System headers first, then project headers
- **Functions**: Keep functions focused and reasonably sized
- **Comments**: Document "why" not "what"

### Memory Management
- **RAII principles** for resource management
- **Arena allocators** for temporary allocations
- **Tagged memory** for debugging and profiling
- **Avoid raw new/delete** in favor of smart pointers or arenas

## Testing

### Running Tests
```bash
# Run all tests
./bin/client/voltrum_client

# Run with validation layers (debug builds)
VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d ./bin/client/voltrum_client
```

### Test Guidelines
- **Unit tests** for individual components
- **Integration tests** for system interactions
- **Visual tests** for rendering features
- **Performance tests** for critical paths

## Documentation

### Code Documentation
- **Document public APIs** with clear descriptions
- **Include usage examples** for complex functions
- **Explain algorithms** and design decisions
- **Keep documentation up-to-date** with code changes

### Architecture Documentation
- See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for system overview
- Update architecture docs when adding new systems
- Document design decisions and trade-offs

## Development Workflow

### Branch Naming
- `feature/feature-name` - New features
- `bugfix/issue-description` - Bug fixes
- `refactor/component-name` - Code refactoring
- `docs/section-name` - Documentation updates

### Commit Messages
Follow conventional commit format:
```
type(scope): brief description

Longer description if needed, explaining the "why" behind the change.

Fixes #123
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

### Review Process
- **All changes** require review from at least one maintainer
- **Address feedback** constructively
- **Keep discussions focused** on technical merit
- **Be patient** - reviews take time

## Recognition

Contributors are recognized in several ways:
- **Contributor list** in README.md
- **Release notes** mention significant contributions
- **GitHub insights** track contributions automatically

## Getting Help

- **GitHub Issues** - For bugs and feature requests
- **GitHub Discussions** - For questions and community chat
- **Code Review** - For feedback on contributions

## License

By contributing to Voltrum, you agree that your contributions will be licensed under the GPL-3.0 License.

---

**Thank you for contributing to Voltrum!**

Your contributions help make this project better for the entire CAD and open-source community.
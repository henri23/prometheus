# Prometheus Architecture

This document provides a comprehensive overview of the Prometheus CAD application architecture, design principles, and system interactions.

## High-Level Architecture

Prometheus follows a **layered architecture** with clear separation between the engine core and application-specific code:

```
┌─────────────────────────────────┐
│        Client Layer             │  ← Application Logic & UI
├─────────────────────────────────┤
│        Core Engine              │  ← Reusable Systems
├─────────────────────────────────┤
│      Platform Layer            │  ← OS Abstraction (SDL3)
├─────────────────────────────────┤
│     Graphics Driver            │  ← Vulkan/GPU
└─────────────────────────────────┘
```

## Core Components

### 1. **Core Engine** (`prometheus_core`)

The core engine provides fundamental systems and abstractions:

#### Application Framework (`core/src/core/`)
- **Entry Point**: Main application lifecycle management
- **Application Class**: Core application loop and subsystem coordination
- **Logger**: Structured logging with multiple output targets

#### Memory Management (`core/src/memory/`)
- **Arena Allocators**: Fast linear allocation for temporary data
- **Memory Pools**: Fixed-size block allocation for objects
- **Tagged Memory**: Debug tracking and leak detection
- **RAII Patterns**: Automatic resource management

#### Platform Layer (`core/src/platform/`)
- **SDL3 Integration**: Cross-platform windowing and input
- **Filesystem**: File I/O abstractions
- **Platform Detection**: OS-specific feature detection

#### Renderer (`core/src/renderer/`)
- **Frontend/Backend Split**: Clean API separation
- **Vulkan Backend**: Modern graphics API implementation
- **Command Buffer Management**: GPU command recording and submission
- **Resource Management**: Buffers, images, and descriptor sets

#### UI System (`core/src/ui/`)
- **ImGui Integration**: Immediate mode GUI framework
- **Dockspace Management**: Window docking and layout
- **Custom Titlebar**: Native-feeling window controls
- **Theme System**: Consistent visual styling

#### Resource System (`core/src/resources/`)
- **Asset Loading**: Images, fonts, and binary data
- **Embedded Resources**: Compile-time asset embedding
- **Resource Registry**: Centralized resource management

### 2. **Client Layer** (`client/`)

Application-specific functionality built on the core engine:

- **UI Components**: CAD-specific interface elements
- **Application Logic**: Microchip design functionality
- **Configuration**: Client-specific settings and initialization

## System Interactions

### Initialization Sequence

```
1. Platform Layer Initialize
   ├── SDL3 Window Creation
   ├── Input System Setup
   └── Vulkan Surface Creation

2. Renderer Initialize
   ├── Vulkan Instance Creation
   ├── Device Selection
   ├── Swapchain Setup
   └── Command Pool Creation

3. Resource Manager Initialize
   ├── Binary Loader Setup
   ├── Image Loader Setup
   └── Asset Registry Creation

4. UI System Initialize
   ├── ImGui Context Creation
   ├── Vulkan Backend Setup
   ├── Font Loading
   └── Dockspace Initialization

5. Client Initialize
   ├── Application-specific Setup
   └── UI Component Registration
```

### Main Loop

```
While Running:
1. Platform Event Processing
   ├── Window Events
   ├── Input Events
   └── System Events

2. Client Update
   ├── Application Logic
   ├── UI State Updates
   └── User Interactions

3. Render Frame
   ├── Command Buffer Recording
   ├── ImGui Rendering
   ├── GPU Command Submission
   └── Present to Screen

4. Resource Management
   ├── Memory Arena Reset
   ├── Temporary Resource Cleanup
   └── Performance Monitoring
```

## Design Principles

### 1. **Separation of Concerns**
- **Clear Boundaries**: Each system has well-defined responsibilities
- **Minimal Dependencies**: Systems depend only on what they need
- **Interface Abstractions**: Implementation details are hidden

### 2. **Performance First**
- **Memory Locality**: Data structures designed for cache efficiency
- **Arena Allocation**: Reduced malloc/free overhead
- **Vulkan Usage**: Modern graphics API for maximum performance

### 3. **Cross-Platform Design**
- **SDL3 Abstraction**: Consistent behavior across platforms
- **Conditional Compilation**: Platform-specific optimizations
- **Standard C++**: Portable language features

### 4. **Modern C++ Practices**
- **RAII**: Automatic resource management
- **Type Safety**: Strong typing and compile-time checks
- **Performance**: Zero-cost abstractions where possible

## Architectural Patterns

### Frontend/Backend Pattern (Renderer)

```cpp
// Frontend: Platform-agnostic interface
struct Renderer_Frontend {
    b8 initialize();
    void render_frame();
    void shutdown();
};

// Backend: Platform-specific implementation
struct Renderer_Backend {
    b8 (*initialize)(Renderer_Backend* backend);
    b8 (*render_frame)(Renderer_Backend* backend);
    void (*shutdown)(Renderer_Backend* backend);
};
```

### Component System (UI)

```cpp
// Base component interface
struct UI_Component {
    virtual void render() = 0;
    virtual void update(f32 delta_time) = 0;
    virtual ~UI_Component() = default;
};

// Specific components
class Titlebar : public UI_Component { /* ... */ };
class Dockspace : public UI_Component { /* ... */ };
```

### Resource Management Pattern

```cpp
// Resource handle system
struct UI_Image_Resource {
    u32 handle;
    u32 width, height;
    void* descriptor_set;  // Vulkan descriptor set
    b8 is_valid;
};

// Resource creation with output parameters
b8 resource_load_image(const char* name, UI_Image_Resource** out_resource);
void resource_free_image(UI_Image_Resource* resource);
```

## Data Flow

### Asset Loading Pipeline

```
Embedded Assets → Binary Loader → Resource Manager → Renderer Backend → GPU Resources
     ↓              ↓                    ↓                ↓               ↓
   Icons,        Raw Data         Resource Handle    Vulkan Objects   GPU Memory
   Fonts         Validation       Generation         Creation         Allocation
```

### Rendering Pipeline

```
Client Update → UI Components → ImGui → Vulkan Backend → GPU
      ↓             ↓            ↓          ↓           ↓
   App Logic    Render Calls   Draw Data  Command      Frame
   Updates      Generation     Creation   Recording    Present
```

### Memory Flow

```
Frame Arena → Temporary Allocations → Reset Each Frame
     ↓
Persistent Arena → Long-lived Objects → Manual Cleanup
     ↓
Pool Allocators → Fixed-size Objects → Automatic Reuse
```

## Key Technologies

### Graphics Stack
- **Vulkan 1.3+**: Modern, low-overhead graphics API
- **SPIRV**: Shader compilation and validation
- **VMA**: Vulkan Memory Allocator for efficient GPU memory management

### UI Stack
- **ImGui**: Immediate mode GUI with docking support
- **SDL3**: Cross-platform windowing and input handling
- **STB Image**: Lightweight image loading and processing

### Build System
- **CMake**: Cross-platform build configuration
- **Ninja**: Fast, parallel build execution
- **Git Submodules**: Dependency management

## Extension Points

### Adding New Renderers
1. Implement `Renderer_Backend` interface
2. Add platform-specific initialization
3. Register with frontend dispatch system

### Adding New UI Components
1. Inherit from base component interface
2. Implement render and update methods
3. Register with UI system

### Adding New Resource Types
1. Create resource loader implementation
2. Add to resource manager registry
3. Implement backend creation functions

## Debugging and Profiling

### Debug Features
- **Vulkan Validation Layers**: GPU debugging and validation
- **Tagged Memory**: Memory leak detection and tracking
- **Comprehensive Logging**: Multi-level debug output
- **Static Assertions**: Compile-time validation

### Performance Monitoring
- **Frame Time Tracking**: Render performance measurement
- **Memory Usage**: Allocation tracking and reporting
- **GPU Profiling**: Vulkan timestamp queries
- **Platform Metrics**: OS-specific performance counters

## Future Architecture Considerations

### Planned Enhancements
- **Multi-threading**: Parallel command buffer recording
- **Asset Streaming**: Dynamic asset loading system
- **Plugin System**: Modular functionality extensions
- **Scripting Integration**: Lua or similar scripting support

### Scalability
- **Scene Graph**: Hierarchical object management
- **Spatial Partitioning**: Efficient spatial queries
- **Level-of-Detail**: Adaptive quality rendering
- **Compute Shaders**: GPU-based computations

---

This architecture provides a solid foundation for a modern CAD application while maintaining flexibility for future enhancements and platform expansion.
cmake_minimum_required(VERSION 3.16)

project(prometheus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SUPPRESS_REGENERATION ON)

# Flexible linking option - default to static for better deployment
option(PROMETHEUS_STATIC_LINKING "Link core and dependencies statically" ON)

# Configure external dependencies based on linking mode
if(PROMETHEUS_STATIC_LINKING)
    message(STATUS "Prometheus: Static linking mode - all libraries will be statically linked")

    # Force SDL3 to build static library only
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)

    # Force spdlog to build static library
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)

else()
    message(STATUS "Prometheus: Dynamic linking mode - using shared libraries where available")

    # Allow SDL3 to build shared library (default behavior)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)

    # Allow spdlog to build shared library
    set(SPDLOG_BUILD_SHARED ON CACHE BOOL "" FORCE)

endif()

# SDL3 configuration
set(SDL_HIDAPI_LIBUSB OFF CACHE BOOL "" FORCE)
set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)      # if you don't need joystick HID
set(SDL_LIBUSB OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)     # don't install SDL system-wide

# External dependencies
add_subdirectory(external/SDL3)
add_subdirectory(external/spdlog)

add_subdirectory(core)

add_subdirectory(client)


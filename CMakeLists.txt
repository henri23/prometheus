cmake_minimum_required(VERSION 3.16)

project(prometheus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SUPPRESS_REGENERATION ON)

set(SDL_HIDAPI_LIBUSB OFF CACHE BOOL "" FORCE)
set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)      # if you don’t need joystick HID
set(SDL_LIBUSB OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)     # don’t install SDL system-wide

# External dependencies
add_subdirectory(external/SDL3)
add_subdirectory(external/spdlog)

add_subdirectory(core)

add_subdirectory(client)


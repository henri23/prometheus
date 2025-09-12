cmake_minimum_required(VERSION 3.16)

project(prometheus)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# External dependencies
add_subdirectory(external/SDL3)

add_subdirectory(core)

add_subdirectory(client)



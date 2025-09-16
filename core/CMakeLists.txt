cmake_minimum_required(VERSION 3.16)
project(prometheus_core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(VULKAN_SDK $ENV{VULKAN_SDK})

add_compile_definitions(API_EXPORT)
add_compile_definitions(DEBUG_BUILD)

# Windows/MSVC specific settings
if(WIN32)
    # Disable some MSVC warnings
    if(MSVC)
        add_compile_options(/W4)  # High warning level
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)  # Disable security warnings for standard C functions
        add_compile_definitions(NOMINMAX)  # Prevent windows.h from defining min/max macros
        add_compile_definitions(WIN32_LEAN_AND_MEAN)  # Reduce Windows header bloat
    endif()
endif()

file(GLOB_RECURSE ENGINE_SRC "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

message(STATUS "Core sources found: ${ENGINE_SRC}")
message(STATUS "Core headers found: ${HEADERS}")

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(spdlog REQUIRED)

# Create the prometheus core .dll library for the time being (maybe switch to static later)
add_library(${PROJECT_NAME} SHARED ${ENGINE_SRC} ${HEADERS})

# Link libraries (Vulkan and SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "src") 
target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_SDK}/Include)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp  # optional
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3 Vulkan::Vulkan)

# Link imgui into core
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)


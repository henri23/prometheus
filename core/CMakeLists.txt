cmake_minimum_required(VERSION 3.16)
project(prometheus_core)

set(CMAKE_CXX_STANDARD 17)

set(VULKAN_SDK $ENV{VULKAN_SDK})

add_compile_definitions(API_EXPORT)
add_compile_definitions(DEBUG_BUILD)

file(GLOB_RECURSE ENGINE_SRC "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

message(STATUS "Core sources found: ${ENGINE_SRC}")
message(STATUS "Core headers found: ${HEADERS}")

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

# Create the prometheus core .dll library for the time being (maybe switch to static later)
add_library(${PROJECT_NAME} SHARED ${ENGINE_SRC} ${HEADERS})

# Link libraries (Vulkan and SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "src") 
target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_SDK}/Include)

# Link with imgui
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

target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

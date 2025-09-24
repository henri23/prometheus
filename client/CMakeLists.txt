cmake_minimum_required(VERSION 3.16)
project(prometheus_client)

set(CMAKE_CXX_STANDARD 17)

# Pass linking mode to client
if(PROMETHEUS_STATIC_LINKING)
    add_compile_definitions(PROMETHEUS_STATIC_LINKING)
else()
    # No API_EXPORT for client - it imports from DLL
endif()

add_compile_definitions(DEBUG_BUILD)

file(GLOB_RECURSE CLIENT_SRC "src/*.cpp")

add_executable(${PROJECT_NAME} ${CLIENT_SRC})

link_directories(${CMAKE_SOURCE_DIR}/core)

target_link_libraries(${PROJECT_NAME} PRIVATE prometheus_core)

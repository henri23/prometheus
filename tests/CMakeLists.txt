cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE TESTS_SRC "src/*.cpp")

# Add test executable
add_executable(prometheus_tests ${TESTS_SRC})

# Link with engine
target_link_libraries(prometheus_tests PRIVATE prometheus_core)

enable_testing()
add_test(prometheus_unit_tests prometheus_tests)


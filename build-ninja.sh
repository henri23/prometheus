#!/bin/bash

# Colors for modern output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Unicode symbols
CHECK_MARK="✓"
CROSS_MARK="✗"
ARROW="→"
STAR="★"
GEAR="⚙"

start_time=$(date +%s%3N)

echo -e "${CYAN}${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}${BOLD}║            ${WHITE}PROMETHEUS BUILD SYSTEM${CYAN}           ║${NC}"
echo -e "${CYAN}${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo

# Function to print colored status messages
print_status() {
    local status=$1
    local message=$2
    case $status in
        "info")
            echo -e "${BLUE}${GEAR}${NC} ${WHITE}${message}${NC}"
            ;;
        "success")
            echo -e "${GREEN}${CHECK_MARK}${NC} ${GREEN}${message}${NC}"
            ;;
        "error")
            echo -e "${RED}${CROSS_MARK}${NC} ${RED}${message}${NC}"
            ;;
        "warning")
            echo -e "${YELLOW}${STAR}${NC} ${YELLOW}${message}${NC}"
            ;;
        "step")
            echo -e "${MAGENTA}${ARROW}${NC} ${WHITE}${message}${NC}"
            ;;
    esac
}

# Function to check if a tool is available
check_tool() {
    local tool=$1
    local package_hint=$2

    if command -v "$tool" >/dev/null 2>&1; then
        print_status "success" "Found $tool"
        return 0
    else
        print_status "error" "Missing required tool: $tool"
        if [ -n "$package_hint" ]; then
            echo -e "   ${YELLOW}${ARROW} Install with: ${WHITE}$package_hint${NC}"
        fi
        return 1
    fi
}

# Tool checking phase
print_status "step" "Checking required build tools..."
echo

tools_ok=true

# Check required tools
# check_tool "cmake" "sudo apt install cmake" || tools_ok=false
# check_tool "ninja" "sudo apt install ninja-build" || tools_ok=false
# check_tool "clang++" "sudo apt install clang" || tools_ok=false
# check_tool "git" "sudo apt install git" || tools_ok=false

echo

if [ "$tools_ok" = false ]; then
    print_status "error" "Some required tools are missing. Please install them before continuing."
    exit 1
fi

print_status "success" "All required tools are available!"
echo

# Ensure the bin directory exists
print_status "step" "Creating build directory..."
mkdir -p bin

# Run CMake with Ninja generator and Clang++
print_status "step" "Configuring project with CMake..."
echo -e "${BLUE}${ARROW} Generator:${NC} Ninja"
echo -e "${BLUE}${ARROW} Compiler:${NC} clang++"
echo -e "${BLUE}${ARROW} Build Type:${NC} Debug"
echo

if time cmake -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -B bin \
    .; then
    print_status "success" "CMake configuration completed"
else
    print_status "error" "CMake configuration failed"
    exit 1
fi
echo

# Go into the bin directory
cd bin

# Link compile_commands.json for IDE integration
print_status "step" "Setting up IDE integration..."
ln -sf "$(pwd)/compile_commands.json" ../compile_commands.json
print_status "success" "Created compile_commands.json symlink"
echo

# Build with Ninja
# echo "=============================================="
# echo "[BUILDER]: Building tests..."
# time ninja koala_tests

# echo "=============================================="
# echo "[BUILDER]: Running tests..."
# ./tests/koala_tests
# TEST_EXIT_CODE=$?
# if [ $TEST_EXIT_CODE -ne 0 ]; then
#     echo "[BUILDER]: Tests failed. Aborting build."
#     exit 1
# fi

print_status "step" "Building prometheus client..."
echo -e "${BLUE}${ARROW} Target:${NC} prometheus_client"
echo

if time ninja prometheus_client; then
    print_status "success" "Build completed successfully"
else
    print_status "error" "Build failed with exit code $?"
    exit 1
fi
echo

# echo "=============================================="
# echo "[BUILDER]: Compiling shaders..."
# sh compile-shaders.sh

# Check for errors
# ERRORLEVEL=$?
# if [ $ERRORLEVEL -ne 0 ]; then
#     print_status "error" "Shader compilation failed with exit code $ERRORLEVEL"
#     exit 1
# fi

# Record end time and calculate duration
end_time=$(date +%s%3N)
tottime=$(expr $end_time - $start_time)

# Convert milliseconds to a more readable format
if [ $tottime -gt 60000 ]; then
    minutes=$((tottime / 60000))
    seconds=$(((tottime % 60000) / 1000))
    ms=$((tottime % 1000))
    time_str="${minutes}m ${seconds}.${ms}s"
elif [ $tottime -gt 1000 ]; then
    seconds=$((tottime / 1000))
    ms=$((tottime % 1000))
    time_str="${seconds}.${ms}s"
else
    time_str="${tottime}ms"
fi

echo
echo -e "${CYAN}${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}${BOLD}║               ${GREEN}BUILD COMPLETED!${CYAN}               ║${NC}"
echo -e "${CYAN}${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo
print_status "success" "All assemblies built successfully"
print_status "info" "Total build time: $time_str"
echo

print_status "step" "Launching prometheus client..."
echo -e "${CYAN}${BOLD}╔══════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}${BOLD}║              ${WHITE}APPLICATION OUTPUT${CYAN}              ║${NC}"
echo -e "${CYAN}${BOLD}╚══════════════════════════════════════════════╝${NC}"
echo

./client/prometheus_client

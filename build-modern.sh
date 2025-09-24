#!/bin/bash

# Modern Build System for Prometheus
# Features: ASCII art, progress tracking, scrolling logs, modern UI

# Usage function
show_usage() {
    echo "Usage: $0 [--dynamic]"
    echo "  --dynamic    Build with dynamic linking (DLL)"
    echo "  (default)    Build with static linking (single executable)"
    exit 1
}

# Parse command line arguments
LINKING_MODE="STATIC"
for arg in "$@"; do
    case $arg in
        --dynamic)
            LINKING_MODE="DYNAMIC"
            shift
            ;;
        --help|-h)
            show_usage
            ;;
        *)
            if [ -n "$arg" ]; then
                echo "Unknown option: $arg"
                show_usage
            fi
            ;;
    esac
done

# Terminal control codes
CLEAR_SCREEN='\033[2J'
CLEAR_LINE='\033[2K'
CURSOR_HOME='\033[H'
HIDE_CURSOR='\033[?25l'
SHOW_CURSOR='\033[?25h'
SAVE_CURSOR='\033[s'
RESTORE_CURSOR='\033[u'

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
BOLD='\033[1m'
DIM='\033[2m'
NC='\033[0m'

# Unicode symbols
CHECK_MARK="✓"
CROSS_MARK="✗"
SPINNER_CHARS="⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"
PROGRESS_FULL="█"
PROGRESS_EMPTY="░"

# Global variables
TERMINAL_HEIGHT=$(tput lines)
TERMINAL_WIDTH=$(tput cols)
LOG_HEIGHT=$((TERMINAL_HEIGHT - 25))  # More space for stable header
STEPS=("Tool Check" "CMake Config" "Build Setup" "Compilation" "Finalization")
CURRENT_STEP=0
CURRENT_PROGRESS=0  # Current progress percentage for smooth transitions
TARGET_PROGRESS=0   # Target progress percentage
STEP_STATUS=("pending" "pending" "pending" "pending" "pending")
LOG_BUFFER=()
LOG_POSITION=0

# Cleanup function
cleanup() {
    echo -e "${SHOW_CURSOR}"
    tput cnorm
    exit
}

trap cleanup EXIT INT TERM

# ASCII Art for PROMETHEUS
show_ascii_art() {
    echo -e "${CYAN}${BOLD}"
    cat << 'EOF'
    ██████╗ ██████╗  ██████╗ ███╗   ███╗███████╗████████╗██╗  ██╗███████╗██╗   ██╗███████╗
    ██╔══██╗██╔══██╗██╔═══██╗████╗ ████║██╔════╝╚══██╔══╝██║  ██║██╔════╝██║   ██║██╔════╝
    ██████╔╝██████╔╝██║   ██║██╔████╔██║█████╗     ██║   ███████║█████╗  ██║   ██║███████╗
    ██╔═══╝ ██╔══██╗██║   ██║██║╚██╔╝██║██╔══╝     ██║   ██╔══██║██╔══╝  ██║   ██║╚════██║
    ██║     ██║  ██║╚██████╔╝██║ ╚═╝ ██║███████╗   ██║   ██║  ██║███████╗╚██████╔╝███████║
    ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚══════╝
EOF
    echo -e "${NC}"
    echo -e "${DIM}                              Modern Build System v2.0${NC}"
    echo
}

# Smooth progress animation - only update progress bar and steps, keep logs stable
animate_progress() {
    while [ $CURRENT_PROGRESS -lt $TARGET_PROGRESS ]; do
        CURRENT_PROGRESS=$((CURRENT_PROGRESS + 1))

        # Move to top and redraw header, progress, and steps only
        printf "${CURSOR_HOME}${HIDE_CURSOR}"
        show_ascii_art
        draw_progress_bar
        echo
        draw_steps

        sleep 0.03  # Smooth animation
    done
}

# Progress bar function
draw_progress_bar() {
    local width=50
    local filled=$((CURRENT_PROGRESS * width / 100))
    local empty=$((width - filled))

    printf "${CYAN}Progress: [${GREEN}"
    for ((i=0; i<filled; i++)); do printf "${PROGRESS_FULL}"; done
    printf "${DIM}"
    for ((i=0; i<empty; i++)); do printf "${PROGRESS_EMPTY}"; done
    printf "${NC}${CYAN}] ${WHITE}%d%%${NC}\n" $CURRENT_PROGRESS
}

# Step status display with fixed layout
draw_steps() {
    echo -e "${WHITE}${BOLD}Build Steps:${NC}"

    # Fixed-width step display to prevent jumping
    for i in "${!STEPS[@]}"; do
        local step="${STEPS[i]}"
        local status="${STEP_STATUS[i]}"
        local step_line=""

        case $status in
            "completed")
                step_line=$(printf "  ${GREEN}${CHECK_MARK} %-20s${NC}" "$step")
                ;;
            "running")
                step_line=$(printf "  ${YELLOW}⚡ %-20s${NC} ${DIM}(in progress)${NC}" "$step")
                ;;
            "failed")
                step_line=$(printf "  ${RED}${CROSS_MARK} %-20s${NC}" "$step")
                ;;
            *)
                step_line=$(printf "  ${DIM}○ %-20s${NC}" "$step")
                ;;
        esac
        echo -e "$step_line"
    done
    echo
}

# Log display with scrolling and fixed height
draw_logs() {
    echo -e "${WHITE}${BOLD}Build Log:${NC}"
    echo -e "${DIM}┌$( printf '%.0s─' $(seq 1 $((TERMINAL_WIDTH-2))) )┐${NC}"

    local start_idx=$((${#LOG_BUFFER[@]} - LOG_HEIGHT))
    if [ $start_idx -lt 0 ]; then start_idx=0; fi

    # Always draw exactly LOG_HEIGHT lines to maintain consistent layout
    for ((i=0; i<LOG_HEIGHT; i++)); do
        local log_idx=$((start_idx + i))
        if [ $log_idx -ge 0 ] && [ $log_idx -lt ${#LOG_BUFFER[@]} ]; then
            local log_line="${LOG_BUFFER[log_idx]}"
            # Truncate long lines to prevent layout issues
            if [ ${#log_line} -gt $((TERMINAL_WIDTH-6)) ]; then
                log_line="${log_line:0:$((TERMINAL_WIDTH-9))}..."
            fi
            printf "${DIM}│${NC} %-$((TERMINAL_WIDTH-4))s ${DIM}│${NC}\n" "$log_line"
        else
            printf "${DIM}│%-$((TERMINAL_WIDTH-2))s│${NC}\n" ""
        fi
    done

    echo -e "${DIM}└$( printf '%.0s─' $(seq 1 $((TERMINAL_WIDTH-2))) )┘${NC}"
}

# Add log entry
add_log() {
    local message="$1"
    local timestamp=$(date '+%H:%M:%S')
    LOG_BUFFER+=("[$timestamp] $message")

    # Keep only last 1000 log entries to prevent memory issues
    if [ ${#LOG_BUFFER[@]} -gt 1000 ]; then
        LOG_BUFFER=("${LOG_BUFFER[@]:100}")
    fi
}

# Static display update (for smooth animation)
update_display_static() {
    local line_num=1

    # Move cursor to home and hide it
    printf "${CURSOR_HOME}${HIDE_CURSOR}"

    # ASCII art (7 lines) + subtitle (2 lines) = 9 lines
    show_ascii_art
    line_num=$((line_num + 9))

    # Progress bar (1 line) + empty line = 2 lines
    draw_progress_bar
    echo
    line_num=$((line_num + 2))

    # Steps (header + 5 steps + empty line) = 7 lines
    draw_steps
    line_num=$((line_num + 7))

    # Log display
    draw_logs
}

# Full display update - clears screen completely (for major updates)
update_display() {
    printf "${CLEAR_SCREEN}${CURSOR_HOME}${HIDE_CURSOR}"
    show_ascii_art
    draw_progress_bar
    echo
    draw_steps
    draw_logs
}

# Set step status with smooth progress
set_step_status() {
    local step_idx=$1
    local status=$2
    STEP_STATUS[$step_idx]=$status

    if [ "$status" = "completed" ]; then
        # Calculate target progress: each step is 20% (100% / 5 steps)
        TARGET_PROGRESS=$(((step_idx + 1) * 20))
        animate_progress
    elif [ "$status" = "running" ]; then
        # Set progress to start of current step + small amount to show activity
        TARGET_PROGRESS=$((step_idx * 20 + 5))
        animate_progress
    fi
}

# Execute command with logging - only update logs section
execute_with_log() {
    local cmd="$1"
    local description="$2"

    add_log "Starting: $description"

    # Only update logs section to avoid flashing
    update_logs_only

    # Execute command and capture output
    local temp_file=$(mktemp)
    if eval "$cmd" > "$temp_file" 2>&1; then
        # Success
        while IFS= read -r line; do
            add_log "$line"
            update_logs_only  # Update logs as we get output
        done < "$temp_file"
        add_log "✓ Completed: $description"
        rm "$temp_file"
        return 0
    else
        # Failure
        while IFS= read -r line; do
            add_log "ERROR: $line"
            update_logs_only
        done < "$temp_file"
        add_log "✗ Failed: $description"
        rm "$temp_file"
        return 1
    fi
}

# Update only the logs section to reduce flashing
update_logs_only() {
    # Calculate the line number where logs start (after header + progress + steps)
    # ASCII art: 7 lines + subtitle: 2 lines + progress: 2 lines + steps: 7 lines = 18 lines
    printf "\033[19;1H"  # Move to line 19 (where logs start)
    draw_logs
}

# Check if a tool is available
check_tool() {
    local tool=$1
    local package_hint=$2

    if command -v "$tool" >/dev/null 2>&1; then
        add_log "✓ Found $tool"
        return 0
    else
        add_log "✗ Missing required tool: $tool"
        if [ -n "$package_hint" ]; then
            add_log "  Install with: $package_hint"
        fi
        return 1
    fi
}

# Main build process
main() {
    # Initialize display
    echo -e "${CLEAR_SCREEN}${CURSOR_HOME}"

    start_time=$(date +%s%3N)

    # Step 1: Tool Check
    set_step_status 0 "running"
    update_display

    add_log "Checking required build tools..."
    tools_ok=true

    check_tool "cmake" "sudo apt install cmake" || tools_ok=false
    check_tool "ninja" "sudo apt install ninja-build" || tools_ok=false
    check_tool "clang++" "sudo apt install clang" || tools_ok=false
    check_tool "git" "sudo apt install git" || tools_ok=false

    if [ "$tools_ok" = false ]; then
        set_step_status 0 "failed"
        add_log "Some required tools are missing. Please install them before continuing."
        update_display
        echo -e "${SHOW_CURSOR}"
        exit 1
    fi

    set_step_status 0 "completed"
    sleep 0.5

    # Step 2: CMake Configuration
    set_step_status 1 "running"

    add_log "Creating build directory..."
    update_logs_only
    mkdir -p bin

    # Set CMake linking option based on mode
    if [ "$LINKING_MODE" = "STATIC" ]; then
        CMAKE_LINKING_FLAG="-DPROMETHEUS_STATIC_LINKING=ON"
        add_log "Linking Mode: Static (self-contained executable)"
        add_log "Libraries: Core, SDL3, spdlog, ImGui → all static"
    else
        CMAKE_LINKING_FLAG="-DPROMETHEUS_STATIC_LINKING=OFF"
        add_log "Linking Mode: Dynamic (executable + shared libraries)"
        add_log "Libraries: Core, SDL3 → shared | spdlog, ImGui → static"
    fi

    if execute_with_log "cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POSITION_INDEPENDENT_CODE=ON $CMAKE_LINKING_FLAG -B bin ." "CMake configuration"; then
        set_step_status 1 "completed"
    else
        set_step_status 1 "failed"
        update_display
        echo -e "${SHOW_CURSOR}"
        exit 1
    fi

    sleep 0.5

    # Step 3: Build Setup
    set_step_status 2 "running"

    cd bin
    add_log "Setting up IDE integration..."
    update_logs_only
    ln -sf "$(pwd)/compile_commands.json" ../compile_commands.json
    add_log "✓ Created compile_commands.json symlink"
    update_logs_only

    set_step_status 2 "completed"
    sleep 0.5

    # Step 4: Compilation
    set_step_status 3 "running"

    if execute_with_log "ninja prometheus_client" "Building prometheus client"; then
        set_step_status 3 "completed"
    else
        set_step_status 3 "failed"
        update_display
        echo -e "${SHOW_CURSOR}"
        exit 1
    fi

    sleep 0.5

    # Step 5: Finalization
    set_step_status 4 "running"

    # Calculate build time
    end_time=$(date +%s%3N)
    tottime=$(expr $end_time - $start_time)

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

    add_log "✓ Build completed successfully in $time_str"
    set_step_status 4 "completed"
    update_display

    # Final success display with logs still visible
    sleep 1

    # Add completion message to logs instead of clearing screen
    add_log ""
    add_log "🎉 BUILD COMPLETED SUCCESSFULLY! 🎉"
    add_log "Build time: $time_str"
    add_log "Executable: ./bin/client/prometheus_client"
    add_log ""
    add_log "Press Enter to launch or Ctrl+C to exit..."

    # Update display one final time to show completion logs
    update_display

    # Move cursor below the log box for input prompt
    echo
    echo -e "${YELLOW}${BOLD}Press Enter to launch the application, or Ctrl+C to exit...${NC}"

    read -r

    echo -e "${CYAN}${BOLD}Launching Prometheus Client...${NC}"
    echo
    ./client/prometheus_client
}

# Start the build process
main
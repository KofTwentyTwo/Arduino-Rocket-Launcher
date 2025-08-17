#!/bin/bash

# 🚀 Luke's Rocket Launch Controller - CMake + PlatformIO Build Script
# Now supports Arduino UNO R3, UNO R4 Minima, and SimulIDE with interactive board selection.

set -e

# Colors for pretty output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project Configuration
PROJECT_NAME="RocketLauncher"
BUILD_DIR="build"
DEFAULT_PRESET="default"
CONFIG_FILE=".board-config"

# Supported board environments
SUPPORTED_BOARDS=("uno_hw" "uno_r4_minima" "simulide")
BOARD_NAMES=("Arduino UNO R3" "Arduino UNO R4 Minima" "Simulation Environment")
BOARD_DESCRIPTIONS=(
    "Classic Arduino (ATmega328P) - Proven reliability, wide compatibility"
    "Modern ARM (Renesas RA4M1) - Enhanced performance, future-proof"
    "Safe testing without hardware - Fast development cycles"
)

# ----------- Pretty printers -----------
print_status()  { echo -e "${GREEN}🔧 $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_error()   { echo -e "${RED}❌ $1${NC}"; }
print_info()    { echo -e "${BLUE}ℹ️  $1${NC}"; }
print_board()   { echo -e "${BLUE}🎯 $1${NC}"; }
print_success() { echo -e "${GREEN}✅ $1${NC}"; }

# ----------- Board Configuration Management -----------
get_board_config() {
    if [[ -f "$CONFIG_FILE" ]]; then
        cat "$CONFIG_FILE"
    else
        echo "uno_r4_minima"  # Default board
    fi
}

set_board_config() {
    local board=$1
    echo "$board" > "$CONFIG_FILE"
    print_success "Board configuration saved: $board"
}

show_board_info() {
    local board=$(get_board_config)
    print_board "Current target board: $board"
    
    case "$board" in
        "uno_hw")
            print_info "  • Arduino UNO R3 (ATmega328P)"
            print_info "  • Classic Arduino compatibility"
            print_info "  • AVR toolchain"
            ;;
        "uno_r4_minima")
            print_info "  • Arduino UNO R4 Minima (Renesas RA4M1)"
            print_info "  • Modern 32-bit ARM architecture"
            print_info "  • Renesas toolchain"
            ;;
        "simulide")
            print_info "  • Simulation environment (AVR)"
            print_info "  • Safe testing without hardware"
            print_info "  • SimulIDE integration"
            ;;
        *)
            print_warning "  • Unknown board environment"
            ;;
    esac
}

validate_board() {
    local board=$1
    for supported in "${SUPPORTED_BOARDS[@]}"; do
        if [[ "$board" == "$supported" ]]; then
            return 0
        fi
    done
    return 1
}

interactive_board_selection() {
    local board_param=$1
    
    # If board parameter is provided, use it directly
    if [[ -n "$board_param" ]]; then
        # Check if it's a valid board name
        if validate_board "$board_param"; then
            print_success "Selected: $board_param"
            
            # Save configuration
            set_board_config "$board_param"
            
            # Show next steps
            echo ""
            print_info "Next steps:"
            case "$board_param" in
                "uno_hw"|"uno_r4_minima")
                    echo "  • Build firmware: ./scripts/build.sh firmware"
                    echo "  • Upload to board: ./scripts/build.sh upload"
                    echo "  • Monitor output: ./scripts/build.sh monitor"
                    ;;
                "simulide")
                    echo "  • Launch simulator: ./scripts/build.sh sim"
                    echo "  • Build simulation: ./scripts/build.sh firmware"
                    ;;
            esac
            return 0
        else
            print_error "Invalid board: $board_param"
            print_info "Available boards:"
            list_supported_boards
            return 1
        fi
    fi
    
    # Interactive selection (original behavior)
    print_status "Board Selection"
    echo ""
    print_info "Please select your target board:"
    echo ""
    
    for i in "${!SUPPORTED_BOARDS[@]}"; do
        local board="${SUPPORTED_BOARDS[$i]}"
        local name="${BOARD_NAMES[$i]}"
        local desc="${BOARD_DESCRIPTIONS[$i]}"
        echo -e "${GREEN}[$((i+1))]${NC} $board - $name"
        echo "     $desc"
        echo ""
    done
    
    local current_board=$(get_board_config)
    local current_index=0
    
    # Find current board index
    for i in "${!SUPPORTED_BOARDS[@]}"; do
        if [[ "${SUPPORTED_BOARDS[$i]}" == "$current_board" ]]; then
            current_index=$((i+1))
            break
        fi
    done
    
    if [[ $current_index -gt 0 ]]; then
        echo -e "${BLUE}Current selection: [$current_index] $current_board${NC}"
        echo ""
    fi
    
    # Get user input
    local choice
    while true; do
        read -p "Enter your choice (1-${#SUPPORTED_BOARDS[@]}): " choice
        
        if [[ "$choice" =~ ^[1-9]$ ]] && [[ "$choice" -le "${#SUPPORTED_BOARDS[@]}" ]]; then
            local selected_board="${SUPPORTED_BOARDS[$((choice-1))]}"
            print_success "Selected: $selected_board"
            
            # Save configuration
            set_board_config "$selected_board"
            
            # Show next steps
            echo ""
            print_info "Next steps:"
            case "$selected_board" in
                "uno_hw"|"uno_r4_minima")
                    echo "  • Build firmware: ./scripts/build.sh firmware"
                    echo "  • Upload to board: ./scripts/build.sh upload"
                    echo "  • Monitor output: ./scripts/build.sh monitor"
                    ;;
                "simulide")
                    echo "  • Launch simulator: ./scripts/build.sh sim"
                    echo "  • Build simulation: ./scripts/build.sh firmware"
                    ;;
            esac
            break
        else
            print_error "Invalid choice. Please enter a number between 1 and ${#SUPPORTED_BOARDS[@]}."
        fi
    done
}

list_supported_boards() {
    print_info "Available board environments:"
    echo ""
    for i in "${!SUPPORTED_BOARDS[@]}"; do
        local board="${SUPPORTED_BOARDS[$i]}"
        local name="${BOARD_NAMES[$i]}"
        local desc="${BOARD_DESCRIPTIONS[$i]}"
        local current=""
        
        if [[ "$board" == "$(get_board_config)" ]]; then
            current=" (current)"
        fi
        
        echo -e "${GREEN}• $board$current${NC}"
        echo "  $name"
        echo "  $desc"
        echo ""
    done
    
    echo -e "${BLUE}Usage examples:${NC}"
    echo "  ./scripts/build.sh firmware    # Build for current board"
    echo "  ./scripts/build.sh upload      # Upload to current board"
    echo "  ./scripts/build.sh monitor     # Monitor current board"
    echo "  ./scripts/build.sh sim         # Launch simulator (if simulation board)"
}

# ----------- Help -----------
show_help() {
    echo -e "${GREEN}🚀 Luke's Rocket Launch Controller - Build System${NC}"
    echo ""
    echo -e "${BLUE}Multi-Board Support:${NC}"
    show_board_info
    echo ""
    echo -e "${BLUE}Board Management:${NC}"
    echo "  ./scripts/build.sh configure  # Configure project and select board"
    echo "  ./scripts/build.sh boards     # List supported boards"
    echo "  ./scripts/build.sh board      # Change board selection"
    echo "  ./scripts/build.sh board uno_r4_minima  # Select board by name"
    echo ""
    echo -e "${BLUE}Available commands:${NC}"
    echo "  build.sh              # Configure + build + tests (default)"
    echo "  build.sh configure    # Configure CMake project and select board"
    echo "  build.sh board        # Change board selection"
    echo "  build.sh build        # Build project"
    echo "  build.sh test         # Run tests"
    echo "  build.sh clean        # Clean build files"
    echo "  build.sh format       # Format source code"
    echo "  build.sh status       # Show project status"
    echo "  build.sh boards       # List supported boards"
    echo ""
    echo -e "${BLUE}Firmware & tooling:${NC}"
    echo "  build.sh firmware     # Build firmware for current board"
    echo "  build.sh sim          # Build + launch SimulIDE (if simulation board)"
    echo "  build.sh upload       # Upload to current board"
    echo "  build.sh monitor      # Open serial monitor for current board"
    echo "  build.sh pio-clean    # Clean PlatformIO files"
    echo ""
    echo -e "${BLUE}Build presets:${NC}"
    echo "  default               # Full build with tests and PlatformIO integration"
    echo "  release               # Optimized release build"
    echo "  tests-only            # Unit tests only (no PlatformIO)"
    echo "  minimal               # Minimal build (no tests, no PlatformIO)"
    echo ""
    echo -e "${BLUE}Examples:${NC}"
    echo "  ./scripts/build.sh"
    echo "  ./scripts/build.sh firmware"
    echo "  ./scripts/build.sh sim"
    echo "  ./scripts/build.sh --preset release"
    echo "  ./scripts/build.sh upload"
    echo "  ./scripts/build.sh monitor"
    echo ""
    echo -e "${BLUE}Board-specific examples:${NC}"
    echo "  # After selecting board with ./scripts/build.sh configure:"
    echo "  ./scripts/build.sh firmware    # Builds for selected board"
    echo "  ./scripts/build.sh upload      # Uploads to selected board"
    echo "  ./scripts/build.sh monitor     # Monitors selected board"
}

# ----------- Dependency checks -----------
check_dependencies() {
    print_status "Checking dependencies..."

    if ! command -v cmake &> /dev/null; then
        print_error "CMake not found. Please install CMake 3.20 or later."
        exit 1
    fi

    if ! command -v make &> /dev/null; then
        print_error "Make not found. Please install Make."
        exit 1
    fi

    if ! command -v pio &> /dev/null; then
        print_warning "PlatformIO not found. Arduino builds/uploads will not work."
        print_info "Install with: pip install platformio"
        print_info "Or use: ./scripts/build.sh --preset tests-only"
    else
        print_status "PlatformIO found"
        
        # Validate current board selection
        local current_board=$(get_board_config)
        if ! validate_board "$current_board"; then
            print_error "Invalid board configuration: $current_board"
            print_info "Please run: ./scripts/build.sh configure"
            exit 1
        fi
        
        # Auto-install renesas-ra if targeting UNO R4 envs
        if [[ "$current_board" == "uno_r4_minima" || "$current_board" == "uno_r4_wifi" ]]; then
            if ! pio platform show renesas-ra &>/dev/null; then
                print_info "Installing PlatformIO renesas-ra toolchain for UNO R4..."
                pio platform install renesas-ra || {
                    print_error "Failed to install renesas-ra platform"
                    exit 1
                }
            fi
        fi
        
        # Auto-install atmelavr if targeting UNO R3 envs
        if [[ "$current_board" == "uno_hw" || "$current_board" == "simulide" ]]; then
            if ! pio platform show atmelavr &>/dev/null; then
                print_info "Installing PlatformIO atmelavr toolchain for UNO R3..."
                pio platform install atmelavr || {
                    print_error "Failed to install atmelavr platform"
                    exit 1
                }
            fi
        fi
    fi

    print_status "Dependencies check complete"
}

# ----------- Presets / build -----------
configure_project() {
    local preset=${1:-$DEFAULT_PRESET}
    print_status "Configuring project with preset: $preset"

    # Check if board is configured
    if [[ ! -f "$CONFIG_FILE" ]]; then
        print_info "No board configuration found. Starting board selection..."
        interactive_board_selection
    fi

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake --preset "$preset" ..
    print_status "Configuration successful"
    cd ..
}

build_project() {
    local preset=${1:-$DEFAULT_PRESET}
    print_status "Building project with preset: $preset"

    if [ ! -d "$BUILD_DIR" ]; then
        print_warning "Build directory not found. Running configure first..."
        configure_project "$preset"
    fi

    cd "$BUILD_DIR"
    cmake --build .
    print_status "Build successful"
    print_info "Output files in: $BUILD_DIR"
    cd ..
}

run_tests() {
    print_status "Running tests..."

    if [ ! -d "$BUILD_DIR" ]; then
        print_warning "Build directory not found. Running configure first..."
        configure_project
    fi

    cd "$BUILD_DIR"
    ctest --output-on-failure
    print_status "Tests completed successfully"
    cd ..
}

clean_build() {
    print_status "Cleaning build files..."

    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_status "Build directory removed"
    fi

    for dir in build-*; do
        if [ -d "$dir" ]; then
            rm -rf "$dir"
            print_status "Removed $dir"
        fi
    done

    print_status "Clean complete"
}



# ----------- Status -----------
show_status() {
    print_status "Project Status"
    print_info "Project: $PROJECT_NAME"
    print_info "Build directory: $BUILD_DIR"
    print_info "Source directory: $(pwd)"

    if [ -d "$BUILD_DIR" ]; then
        print_info "✅ Build directory exists"
        if [ -f "$BUILD_DIR/rocket_tests" ]; then
            print_info "✅ Test executable exists"
        fi
    else
        print_warning "❌ Build directory missing"
    fi

    if command -v pio &> /dev/null; then
        print_info "✅ PlatformIO found"
        if [ -f "platformio.ini" ]; then
            print_info "✅ PlatformIO project configuration found"
        else
            print_warning "⚠️  PlatformIO project configuration not found"
        fi
        
        # Show current board information
        echo ""
        show_board_info
        
        # Show available platforms
        echo ""
        print_info "Installed PlatformIO platforms:"
        if pio platform show atmelavr &>/dev/null; then
            print_info "  ✅ atmelavr (for UNO R3 and simulation)"
        else
            print_warning "  ❌ atmelavr (required for UNO R3 and simulation)"
        fi
        
        if pio platform show renesas-ra &>/dev/null; then
            print_info "  ✅ renesas-ra (for UNO R4 Minima)"
        else
            print_warning "  ❌ renesas-ra (required for UNO R4 Minima)"
        fi
        
        # Show available devices
        echo ""
        print_info "Available devices:"
        if pio device list &>/dev/null; then
            pio device list --serial | head -10
        else
            print_warning "No devices found"
        fi
    else
        print_warning "❌ PlatformIO not found"
    fi
}

# ----------- Main -----------
main() {
    local command=${1:-"all"}
    local preset=${2:-$DEFAULT_PRESET}

    case $command in
        "help"|"-h"|"--help") show_help ;;

        "configure")  check_dependencies ; configure_project "$preset" ;;
        "build")      check_dependencies ; build_project "$preset" ;;
        "test")       check_dependencies ; run_tests ;;
        "clean")      clean_build ;;
        "format")     format_code ;;
        "status")     show_status ;;
        "boards")     list_supported_boards ;;
        "board")      interactive_board_selection "$2" ;;

        "firmware")   check_dependencies ; pio_build_firmware ;;
        "sim")        launch_simulator ;;

        "upload")     check_dependencies ; pio_upload ;;
        "monitor")    check_dependencies ; pio_monitor ;;
        "pio-clean")  check_dependencies ; pio_clean ;;

        "all") check_dependencies ; configure_project "$preset" ; build_project "$preset" ; run_tests ;;
        *) print_error "Unknown command: $command" ; show_help ; exit 1 ;;
    esac
}

# ----------- PlatformIO helpers (Multi-board safe) -----------
resolve_pio_env() {
    local env_name=$(get_board_config)
    
    # Validate the environment
    if ! validate_board "$env_name"; then
        print_error "Invalid board configuration: $env_name"
        list_supported_boards
        exit 1
    fi
    
    echo "$env_name"
}

detect_port() {
    # Prefer PlatformIO's device listing (portable across OSes)
    local port
    port=$(pio device list --serial | awk -F': ' '/Port: /{print $2}' | head -n1)
    echo "$port"
}

pio_build_firmware() {
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO is required for firmware building"
        exit 1
    fi
    
    local env_name
    env_name="$(resolve_pio_env)"
    print_status "Building firmware with PlatformIO (env: $env_name)"
    
    # Show board info before build
    show_board_info

    # Build the firmware
    pio run -e "$env_name"
    
    print_success "Firmware built successfully for $env_name"
}

pio_upload() {
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO is required for upload"
        exit 1
    fi
    
    local env_name
    env_name="$(resolve_pio_env)"
    print_status "Uploading with PlatformIO (env: $env_name)"
    
    # Show board info before upload
    show_board_info

    local port
    port="$(detect_port)"
    if [[ -n "$port" ]]; then
        print_info "Using serial port: $port"
        pio run -t upload -e "$env_name" --upload-port "$port"
    else
        print_warning "No serial port detected, attempting auto-detect..."
        pio run -t upload -e "$env_name"
    fi
}

pio_monitor() {
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO is required for monitor"
        exit 1
    fi
    
    local env_name baud port
    env_name="$(resolve_pio_env)"
    baud="115200"
    port="$(detect_port)"

    # Show board info before monitor
    show_board_info

    if [[ -n "$port" ]]; then
        print_status "Opening serial monitor @ ${baud} on ${port} (env: $env_name)"
        pio device monitor --baud "$baud" --port "$port"
    else
        print_status "Opening serial monitor @ ${baud} (env: $env_name)"
        print_warning "No specific port detected, using environment default"
        pio device monitor --baud "$baud" -e "$env_name"
    fi
}

pio_clean() {
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO is required for cleaning"
        exit 1
    fi
    
    local env_name
    env_name="$(resolve_pio_env)"
    print_status "Cleaning PlatformIO files (env: $env_name)"
    
    # Show board info before clean
    show_board_info

    # Clean the environment
    pio run -t clean -e "$env_name"
    
    print_success "PlatformIO files cleaned for $env_name"
}

# ----------- SimulIDE flow (Multi-board aware) -----------
launch_simulator() {
    print_status "Building and launching simulator..."
    
    # Ensure we're using the simulation environment
    local current_board=$(get_board_config)
    if [[ "$current_board" != "simulide" ]]; then
        print_warning "Current board is $current_board, but simulation requires 'simulide' environment"
        print_info "Switching to simulation environment for this operation..."
        set_board_config "simulide"
    fi

    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO not found. Cannot build simulation firmware."
        print_info "Install with: pip install platformio"
        exit 1
    fi

    print_info "Building simulation firmware..."
    pio run -e simulide

    print_status "Simulation firmware built successfully"

    BUILD_FIRMWARE_PATH=".pio/build/simulide/firmware.hex"
    EXPECTED_FIRMWARE_PATH="out/simulide/firmware.hex"

    if [[ ! -f "$BUILD_FIRMWARE_PATH" ]]; then
        print_error "Firmware file not found at build path: $BUILD_FIRMWARE_PATH"
        exit 1
    fi

    print_info "Copying firmware to expected location..."
    mkdir -p "out/simulide"
    cp "$BUILD_FIRMWARE_PATH" "$EXPECTED_FIRMWARE_PATH"

    print_status "Firmware ready at: $EXPECTED_FIRMWARE_PATH"

    SIMULIDE_PATH=""
    if [[ "$OSTYPE" == "darwin"* ]]; then
        if [[ -d "/Applications/simulide.app" ]]; then
            SIMULIDE_PATH="/Applications/simulide.app/Contents/MacOS/simulide"
        elif [[ -d "$HOME/Applications/simulide.app" ]]; then
            SIMULIDE_PATH="$HOME/Applications/simulide.app/Contents/MacOS/simulide"
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v simulide &> /dev/null; then
            SIMULIDE_PATH="simulide"
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        if command -v simulide.exe &> /dev/null; then
            SIMULIDE_PATH="simulide.exe"
        fi
    fi

    if [[ -n "$SIMULIDE_PATH" ]]; then
        print_status "Launching SimulIDE..."
        WIRING_FILE="../../wiring/rocker_launcher_controls.sim1"
        if [[ ! -f "$WIRING_FILE" ]]; then
            print_warning "Default wiring file not found, looking for alternatives..."
            ALTERNATIVE_WIRING=$(find "../../wiring" -name "*.sim1" | head -1)
            if [[ -n "$ALTERNATIVE_WIRING" ]]; then
                WIRING_FILE="$ALTERNATIVE_WIRING"
                print_info "Using alternative wiring file: $WIRING_FILE"
            else
                print_warning "No wiring files found, launching SimulIDE without wiring file"
                WIRING_FILE=""
            fi
        fi

        if [[ -n "$WIRING_FILE" ]]; then
            print_info "Loading wiring file: $WIRING_FILE"
            "$SIMULIDE_PATH" "$WIRING_FILE" &
        else
            print_info "Launching SimulIDE without wiring file..."
            "$SIMULIDE_PATH" &
        fi

        print_status "SimulIDE launched successfully!"
        print_info "💡 Tips:"
        print_info "  • The firmware will auto-load in SimulIDE"
        print_info "  • Make changes and re-run: ./scripts/build.sh sim"
        print_info "  • Check SimulIDE console for runtime errors"
        print_info "  • Use ./scripts/build.sh board to change board selection"
    else
        print_warning "SimulIDE not found - firmware built but not launched"
        print_info "📋 Manual Launch:"
        print_info "1. Open SimulIDE"
        print_info "2. Load: ../../wiring/rocker_launcher_controls.sim1"
        print_info "3. Firmware: $EXPECTED_FIRMWARE_PATH"
    fi

    print_status "Simulator setup complete!"
}

# ----------- Optional: code formatter -----------
format_code() {
    print_status "Formatting source code..."
    if command -v clang-format &> /dev/null; then
        find src/ -name "*.cpp" -o -name "*.h" | xargs clang-format -i
        print_status "Code formatting complete"
    else
        print_warning "clang-format not found. Install clang-format to format code."
        print_info "macOS: brew install clang-format"
        print_info "Ubuntu: sudo apt install clang-format"
    fi
}

# ----------- Preset flag handling -----------
if [[ "$1" == "--preset" ]]; then
    preset="$2"
    shift 2
    main "$1" "$preset"
else
    main "$@"
fi

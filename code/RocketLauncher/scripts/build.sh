#!/bin/bash

# 🚀 Luke's Rocket Launch Controller - CMake Build Script
# Modern build system for Arduino-based rocket launcher with PlatformIO integration

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

# Function to print colored output
print_status() {
    echo -e "${GREEN}🔧 $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Function to show help
show_help() {
    echo -e "${GREEN}🚀 Luke's Rocket Launch Controller - Build System${NC}"
    echo ""
    echo -e "${BLUE}Available commands:${NC}"
    echo "  build.sh              # Configure and build everything (default)"
    echo "  build.sh configure    # Configure CMake project"
    echo "  build.sh build        # Build project"
    echo "  build.sh test         # Run tests"
    echo "  build.sh clean        # Clean build files"
    echo "  build.sh format       # Format source code"
    echo "  build.sh status       # Show project status"
    echo ""
    echo -e "${BLUE}CMake targets (what CLion sees):${NC}"
    echo "  build.sh firmware-sim # Build simulation firmware"
    echo "  build.sh firmware-hw  # Build hardware firmware"
    echo "  build.sh all-firmware # Build both firmwares"
    echo "  build.sh sim          # Build and launch simulator"
    echo "  build.sh upload       # Upload to Arduino"
    echo "  build.sh monitor      # Start serial monitor"
    echo "  build.sh pio-clean    # Clean PlatformIO files"
    echo ""
    echo -e "${BLUE}Build presets:${NC}"
    echo "  default               # Full build with tests and PlatformIO integration"
    echo "  release               # Optimized release build"
    echo "  tests-only            # Unit tests only (no PlatformIO)"
    echo "  minimal               # Minimal build (no tests, no PlatformIO)"
    echo ""
    echo -e "${BLUE}Examples:${NC}"
    echo "  ./scripts/build.sh                    # Full build"
    echo "  ./scripts/build.sh firmware-sim       # Build simulation firmware"
    echo "  ./scripts/build.sh sim                # Build and launch simulator"
    echo "  ./scripts/build.sh --preset release   # Release build"
}

# Function to check dependencies
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
    
    # Check for PlatformIO
    if ! command -v pio &> /dev/null; then
        print_warning "PlatformIO not found. Arduino builds will not work."
        print_info "Install with: pip install platformio"
        print_info "Or use: ./scripts/build.sh --preset tests-only"
    else
        print_status "PlatformIO found"
    fi
    
    print_status "Dependencies check complete"
}

# Function to configure project
configure_project() {
    local preset=${1:-$DEFAULT_PRESET}
    print_status "Configuring project with preset: $preset"
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake --preset "$preset" ..
    
    if [ $? -eq 0 ]; then
        print_status "Configuration successful"
    else
        print_error "Configuration failed"
        exit 1
    fi
    
    cd ..
}

# Function to build project
build_project() {
    local preset=${1:-$DEFAULT_PRESET}
    print_status "Building project with preset: $preset"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_warning "Build directory not found. Running configure first..."
        configure_project "$preset"
    fi
    
    cd "$BUILD_DIR"
    # Build without preset since we're already in the configured build directory
    cmake --build .
    
    if [ $? -eq 0 ]; then
        print_status "Build successful"
        print_info "Output files in: $BUILD_DIR"
    else
        print_error "Build failed"
        exit 1
    fi
    
    cd ..
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_warning "Build directory not found. Running configure first..."
        configure_project
    fi
    
    cd "$BUILD_DIR"
    # Run tests without preset since we're already in the configured build directory
    ctest --output-on-failure
    
    if [ $? -eq 0 ]; then
        print_status "Tests completed successfully"
    else
        print_error "Tests failed"
        exit 1
    fi
    
    cd ..
}

# Function to clean build files
clean_build() {
    print_status "Cleaning build files..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_status "Build directory removed"
    fi
    
    # Clean other build directories
    for dir in build-*; do
        if [ -d "$dir" ]; then
            rm -rf "$dir"
            print_status "Removed $dir"
        fi
    done
    
    print_status "Clean complete"
}

# Function to build specific CMake targets
build_target() {
    local target=$1
    local description=$2
    
    print_status "$description"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_warning "Build directory not found. Running configure first..."
        configure_project
    fi
    
    cd "$BUILD_DIR"
    cmake --build . --target "$target"
    
    if [ $? -eq 0 ]; then
        print_status "Target '$target' built successfully"
    else
        print_error "Target '$target' build failed"
        exit 1
    fi
    
    cd ..
}

# Function to format code
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

# Function to build and launch simulator
launch_simulator() {
    print_status "Building and launching simulator..."
    
    # Check if PlatformIO is available
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO not found. Cannot build simulator firmware."
        print_info "Install with: pip install platformio"
        exit 1
    fi
    
    # Build simulation firmware
    print_info "Building simulation firmware..."
    pio run -e simulide
    
    if [ $? -ne 0 ]; then
        print_error "Failed to build simulation firmware"
        exit 1
    fi
    
    print_status "Simulation firmware built successfully"
    
    # Check if firmware was created and copy to expected location
    BUILD_FIRMWARE_PATH=".pio/build/simulide/firmware.hex"
    EXPECTED_FIRMWARE_PATH="out/simulide/firmware.hex"
    
    if [[ ! -f "$BUILD_FIRMWARE_PATH" ]]; then
        print_error "Firmware file not found at build path: $BUILD_FIRMWARE_PATH"
        exit 1
    fi
    
    # Create output directory and copy firmware (same as PlatformIO upload_command)
    print_info "Copying firmware to expected location..."
    mkdir -p "out/simulide"
    cp "$BUILD_FIRMWARE_PATH" "$EXPECTED_FIRMWARE_PATH"
    
    if [[ ! -f "$EXPECTED_FIRMWARE_PATH" ]]; then
        print_error "Failed to copy firmware to expected location"
        exit 1
    fi
    
    print_status "Firmware ready at: $EXPECTED_FIRMWARE_PATH"
    
    # Check if SimulIDE is installed and launch it
    SIMULIDE_PATH=""
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        if [[ -d "/Applications/simulide.app" ]]; then
            SIMULIDE_PATH="/Applications/simulide.app/Contents/MacOS/simulide"
        elif [[ -d "$HOME/Applications/simulide.app" ]]; then
            SIMULIDE_PATH="$HOME/Applications/simulide.app/Contents/MacOS/simulide"
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
        if command -v simulide &> /dev/null; then
            SIMULIDE_PATH="simulide"
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        # Windows
        if command -v simulide.exe &> /dev/null; then
            SIMULIDE_PATH="simulide.exe"
        fi
    fi
    
    if [[ -n "$SIMULIDE_PATH" ]]; then
        print_status "Launching SimulIDE..."
        
        # Choose the wiring file to use
        WIRING_FILE="../../wiring/rocker_launcher_controls.sim1"
        if [[ ! -f "$WIRING_FILE" ]]; then
            print_warning "Default wiring file not found, looking for alternatives..."
            
            # Find any .sim1 file
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
        print_info "💡 Development Tips:"
        print_info "  • The firmware will auto-load in SimulIDE"
        print_info "  • Make changes to your code and re-run: ./scripts/build.sh sim"
        print_info "  • Check SimulIDE console for any runtime errors"
    else
        print_warning "SimulIDE not found - firmware built but not launched"
        print_info "📋 Manual Launch Instructions:"
        print_info "1. Open SimulIDE manually"
        print_info "2. Load the wiring file: ../../wiring/rocker_launcher_controls.sim1"
        print_info "3. The firmware will auto-load from: $EXPECTED_FIRMWARE_PATH"
        print_info ""
        print_info "Install SimulIDE from: https://www.simulide.com/"
    fi
    
    print_status "Simulator setup complete!"
}

# Function to show status
show_status() {
    print_status "Project Status"
    print_info "Project: $PROJECT_NAME"
    print_info "Build directory: $BUILD_DIR"
    print_info "Source directory: $(pwd)"
    
    if [ -d "$BUILD_DIR" ]; then
        print_info "✅ Build directory exists"
        
        # Check for test executable
        if [ -f "$BUILD_DIR/rocket_tests" ]; then
            print_info "✅ Test executable exists"
        fi
    else
        print_warning "❌ Build directory missing"
    fi
    
    # Check PlatformIO status
    if command -v pio &> /dev/null; then
        print_info "✅ PlatformIO found"
        
        # Check PlatformIO project
        if [ -f "platformio.ini" ]; then
            print_info "✅ PlatformIO project configuration found"
        else
            print_warning "⚠️  PlatformIO project configuration not found"
        fi
    else
        print_warning "❌ PlatformIO not found"
    fi
}

# Main script logic
main() {
    local command=${1:-"all"}
    local preset=${2:-$DEFAULT_PRESET}
    
    case $command in
        "help"|"-h"|"--help")
            show_help
            exit 0
            ;;
        "configure")
            check_dependencies
            configure_project "$preset"
            ;;
        "build")
            check_dependencies
            build_project "$preset"
            ;;
        "test")
            check_dependencies
            run_tests
            ;;
        "clean")
            clean_build
            ;;
        "format")
            format_code
            ;;
        "status")
            show_status
            ;;
        "firmware-sim")
            check_dependencies
            build_target "firmware_sim" "Building simulation firmware"
            ;;
        "firmware-hw")
            check_dependencies
            build_target "firmware_hw" "Building hardware firmware"
            ;;
        "all-firmware")
            check_dependencies
            build_target "all_firmware" "Building both firmwares"
            ;;
        "sim")
            launch_simulator
            ;;
        "upload")
            check_dependencies
            build_target "upload" "Building and uploading firmware"
            ;;
        "monitor")
            check_dependencies
            build_target "monitor" "Starting serial monitor"
            ;;
        "pio-clean")
            check_dependencies
            build_target "pio_clean" "Cleaning PlatformIO files"
            ;;
        "all")
            check_dependencies
            configure_project "$preset"
            build_project "$preset"
            run_tests
            ;;
        *)
            print_error "Unknown command: $command"
            show_help
            exit 1
            ;;
    esac
}

# Handle preset argument
if [[ "$1" == "--preset" ]]; then
    preset="$2"
    shift 2
    main "$1" "$preset"
else
    main "$@"
fi

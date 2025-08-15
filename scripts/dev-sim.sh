#!/bin/bash

# 🚀 Luke's Rocket Launcher - Development Simulator Script
# This script builds and launches the simulator for local development

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ROCKET_DIR="$PROJECT_DIR/code/RocketLauncher"

echo -e "${BLUE}🚀 Luke's Rocket Launcher - Development Simulator${NC}"
echo -e "${BLUE}================================================${NC}"
echo ""

# Check if we're in the right directory
if [[ ! -f "$ROCKET_DIR/platformio.ini" ]]; then
    echo -e "${RED}❌ Error: platformio.ini not found${NC}"
    echo "Please run this script from the project root directory"
    exit 1
fi

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo -e "${RED}❌ Error: PlatformIO not found${NC}"
    echo "Please install PlatformIO: pip install platformio"
    exit 1
fi

# Check if SimulIDE is installed
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

if [[ -z "$SIMULIDE_PATH" ]]; then
    echo -e "${YELLOW}⚠️  Warning: SimulIDE not found${NC}"
    echo "The simulator will build but won't auto-launch"
    echo "Please install SimulIDE from: https://www.simulide.com/"
    echo ""
fi

# Change to RocketLauncher directory
cd "$ROCKET_DIR"

echo -e "${BLUE}📁 Working directory: $(pwd)${NC}"
echo ""

# Clean previous builds (optional - can be skipped for faster iteration)
if [[ "$1" == "--clean" ]]; then
    echo -e "${BLUE}🧹 Cleaning previous builds...${NC}"
    pio run -t clean
    echo ""
fi

# Build the simulation firmware
echo -e "${BLUE}🔨 Building simulation firmware...${NC}"
pio run -e simulide

if [[ $? -eq 0 ]]; then
    echo -e "${GREEN}✅ Simulation firmware built successfully!${NC}"
else
    echo -e "${RED}❌ Failed to build simulation firmware${NC}"
    exit 1
fi

echo ""

# Check if firmware was created and copy to expected location
BUILD_FIRMWARE_PATH="$ROCKET_DIR/.pio/build/simulide/firmware.hex"
EXPECTED_FIRMWARE_PATH="$ROCKET_DIR/out/simulide/firmware.hex"

if [[ ! -f "$BUILD_FIRMWARE_PATH" ]]; then
    echo -e "${RED}❌ Error: Firmware file not found at build path${NC}"
    echo "Expected: $BUILD_FIRMWARE_PATH"
    echo "This might be a PlatformIO build issue"
    exit 1
fi

# Create output directory and copy firmware (same as PlatformIO upload_command)
echo -e "${BLUE}📁 Copying firmware to expected location...${NC}"
mkdir -p "$ROCKET_DIR/out/simulide"
cp "$BUILD_FIRMWARE_PATH" "$EXPECTED_FIRMWARE_PATH"

if [[ ! -f "$EXPECTED_FIRMWARE_PATH" ]]; then
    echo -e "${RED}❌ Error: Failed to copy firmware to expected location${NC}"
    exit 1
fi

echo -e "${GREEN}📁 Firmware ready at: $EXPECTED_FIRMWARE_PATH${NC}"
echo ""

# Launch SimulIDE if available
if [[ -n "$SIMULIDE_PATH" ]]; then
    echo -e "${BLUE}🚀 Launching SimulIDE...${NC}"
    
    # Choose the wiring file to use
    WIRING_FILE="$PROJECT_DIR/wiring/rocker_launcher_controls.sim1"
    if [[ ! -f "$WIRING_FILE" ]]; then
        echo -e "${YELLOW}⚠️  Warning: Default wiring file not found${NC}"
        echo "Looking for alternative wiring files..."
        
        # Find any .sim1 file
        ALTERNATIVE_WIRING=$(find "$PROJECT_DIR/wiring" -name "*.sim1" | head -1)
        if [[ -n "$ALTERNATIVE_WIRING" ]]; then
            WIRING_FILE="$ALTERNATIVE_WIRING"
            echo -e "${GREEN}✅ Using alternative wiring file: $WIRING_FILE${NC}"
        else
            echo -e "${RED}❌ No wiring files found in $PROJECT_DIR/wiring${NC}"
            echo "SimulIDE will launch without a wiring file"
            WIRING_FILE=""
        fi
    fi
    
    if [[ -n "$WIRING_FILE" ]]; then
        echo -e "${BLUE}🔌 Loading wiring file: $WIRING_FILE${NC}"
        "$SIMULIDE_PATH" "$WIRING_FILE" &
    else
        echo -e "${BLUE}🚀 Launching SimulIDE without wiring file...${NC}"
        "$SIMULIDE_PATH" &
    fi
    
    echo -e "${GREEN}✅ SimulIDE launched!${NC}"
    echo ""
    echo -e "${BLUE}💡 Development Tips:${NC}"
    echo "  • The firmware will auto-load in SimulIDE"
    echo "  • Make changes to your code and re-run this script"
    echo "  • Use '--clean' flag to force a full rebuild"
    echo "  • Check SimulIDE console for any runtime errors"
else
    echo -e "${YELLOW}⚠️  SimulIDE not available - firmware built but not launched${NC}"
    echo ""
    echo -e "${BLUE}📋 Manual Launch Instructions:${NC}"
    echo "1. Open SimulIDE manually"
    echo "2. Load the wiring file: $PROJECT_DIR/wiring/rocker_launcher_controls.sim1"
    echo "3. The firmware will auto-load from: $EXPECTED_FIRMWARE_PATH"
fi

echo ""
echo -e "${GREEN}🎉 Development environment ready!${NC}"
echo ""
echo -e "${BLUE}📚 Quick Commands:${NC}"
echo "  • Rebuild & launch: $0"
echo "  • Clean rebuild: $0 --clean"
echo "  • Run tests: cd $ROCKET_DIR && pio test -e native"
echo "  • Build hardware: cd $ROCKET_DIR && pio run -e uno_hw"
echo ""

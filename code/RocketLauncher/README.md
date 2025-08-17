# 🚀 Luke's Rocket Launch Controller

A modern Arduino-based rocket launcher controller built with **CMake** and **PlatformIO**.

## 🏗️ Build System

This project uses a **hybrid build approach** that gives you the best of both worlds:

- **CMake** - Primary build system for IDE integration (CLion, VS Code) and project management
- **PlatformIO** - Arduino firmware building and library management
- **Unity** - Unit testing framework for native development

### Key Benefits

- **CLion Tier 1 Support** - Full CMake integration for the best IDE experience
- **Cross-platform compatibility** - Works on macOS, Linux, and Windows
- **Multiple build configurations** - Presets for different use cases
- **Modern C++ standards** - C++17 support with proper toolchain
- **Maintainable architecture** - Standard CMake practices
- **Multi-board support** - Arduino UNO R3, UNO R4 Minima, and simulation

## 🎯 Multi-Board Support

This project supports multiple Arduino boards, allowing you to choose your target hardware at build time.

### Supported Boards

| Board | Platform | MCU | Framework | Upload Protocol |
|-------|----------|-----|-----------|------------------|
| **Arduino UNO R3** | `uno_hw` | ATmega328P | Arduino AVR | Serial/USB |
| **Arduino UNO R4 Minima** | `uno_r4_minima` | Renesas RA4M1 | Arduino Renesas | Serial/USB |
| **Simulation** | `simulide` | AVR Simulated | Arduino AVR | Custom (SimulIDE) |

### Board Selection

#### **Quick Board Selection**
```bash
# Select your target board interactively
./scripts/build.sh configure

# Change board selection anytime
./scripts/build.sh board

# Quick board selection by name
./scripts/build.sh board uno_hw        # Arduino UNO R3
./scripts/build.sh board uno_r4_minima # Arduino UNO R4 Minima
./scripts/build.sh board simulide      # Simulation

# Show current board
./scripts/build.sh status
```

#### Method 2: PlatformIO Direct Commands
```bash
# Build for specific board
pio run -e uno_hw
pio run -e uno_r4_minima
pio run -e simulide

# Upload to specific board
pio run -e uno_hw -t upload
pio run -e uno_r4_minima -t upload

# Monitor specific board
pio device monitor -e uno_hw
pio device monitor -e uno_r4_minima
```

#### Method 3: Build Script with Board Selection
```bash
# The build script automatically uses your configured board
./scripts/build.sh firmware    # Builds for current board
./scripts/build.sh upload      # Uploads to current board
./scripts/build.sh monitor     # Monitors current board

# All commands respect your board selection
```

### Board-Specific Features

#### Arduino UNO R3 (ATmega328P)
- **Classic Arduino compatibility** - Works with all standard Arduino libraries
- **Proven reliability** - Battle-tested platform for rocketry
- **Wide library support** - Maximum compatibility with existing code
- **Lower cost** - More affordable hardware option

#### Arduino UNO R4 Minima (Renesas RA4M1)
- **Modern architecture** - 32-bit ARM Cortex-M4 processor
- **Higher performance** - Faster execution and more memory
- **Enhanced features** - Better analog capabilities and I/O
- **Future-proof** - Next-generation Arduino platform

#### Simulation Environment
- **Safe testing** - Test firmware without physical hardware
- **Fast iteration** - Quick development and debugging cycles
- **Cost-effective** - No hardware costs during development
- **Educational** - Great for learning and demonstration

### Board Switching Workflow

1. **Configure your project** - `./scripts/build.sh configure` (selects board)
2. **Build and test** - `./scripts/build.sh firmware` (uses selected board)
3. **Change board anytime** - `./scripts/build.sh board` (new selection)
4. **Verify functionality** - All commands automatically use current board

### Example Development Workflow

```bash
# 1. Start with simulation (safe testing)
./scripts/build.sh configure    # Select simulide
./scripts/build.sh sim

# 2. Test on UNO R3 (classic hardware)
./scripts/build.sh board        # Select uno_hw
./scripts/build.sh firmware
./scripts/build.sh upload
./scripts/build.sh monitor

# 3. Test on UNO R4 Minima (modern hardware)
./scripts/build.sh board        # Select uno_r4_minima
./scripts/build.sh firmware
./scripts/build.sh upload
./scripts/build.sh monitor

# 4. Compare behavior across platforms
# All boards should provide identical functionality
```

### Troubleshooting Multi-Board Issues

#### Common Problems and Solutions

**Library Compatibility Issues**
```bash
# If libraries don't work on a specific board
pio lib install "library_name" -e uno_hw
pio lib install "library_name" -e uno_r4_minima
```

**Upload Failures**
```bash
# Check available ports
pio device list

# Specify exact port
pio run -e uno_hw -t upload --upload-port /dev/cu.usbmodemXXXX
```

**Build Errors**
```bash
# Clean and rebuild
pio run -t clean -e uno_hw
pio run -e uno_hw

# Check board-specific requirements
pio platform show renesas-ra
pio platform show atmelavr
```

## 📋 Prerequisites

### Required
- **CMake 3.20+** - Modern build system
- **Make** - Build tool (usually comes with CMake)
- **C++17 compatible compiler** - For native builds and testing
- **PlatformIO** - For Arduino development

### Optional
- **SimulIDE** - For simulation testing
- **clang-format** - For code formatting

### Installation

#### macOS
```bash
# Install CMake
brew install cmake

# Install PlatformIO
pip install platformio

# Install SimulIDE (optional)
brew install --cask simulide

# Install clang-format (optional)
brew install clang-format
```

#### Ubuntu/Debian
```bash
# Install CMake and build tools
sudo apt update
sudo apt install cmake build-essential

# Install PlatformIO
pip install platformio

# Install SimulIDE (optional)
sudo apt install simulide

# Install clang-format (optional)
sudo apt install clang-format
```

#### Windows
- Download and install [CMake](https://cmake.org/download/)
- Install PlatformIO: `pip install platformio`
- Download and install [SimulIDE](https://www.simulide.com/) (optional)

## 🚀 Quick Start

### 1. Clone and Navigate
```bash
git clone <your-repo-url>
cd RocketLauncher
```

### 2. Choose Your Target Board
```bash
# For Arduino UNO R3 (classic)
PIO_ENV=uno_hw ./scripts/build.sh

# For Arduino UNO R4 Minima (modern) - DEFAULT
./scripts/build.sh

# For simulation only
PIO_ENV=simulide ./scripts/build.sh sim
```

### 3. Build Everything
```bash
# Using the build script (recommended)
./scripts/build.sh

# Or using CMake directly
cmake --preset default
cmake --build --preset default
```

### 4. Run Tests
```bash
./scripts/build.sh test
```

## 🛠️ Build Commands

### Using the Build Script (Recommended)

The `scripts/build.sh` script provides a familiar interface with multi-board support:

```bash
# Full build (configure + build + test) - uses default board
./scripts/build.sh

# Build for specific board
PIO_ENV=uno_hw ./scripts/build.sh
PIO_ENV=uno_r4_minima ./scripts/build.sh
PIO_ENV=simulide ./scripts/build.sh

# Configure only
./scripts/build.sh configure

# Build only (assumes already configured)
./scripts/build.sh build

# Run tests
./scripts/build.sh test

# Clean build files
./scripts/build.sh clean

# Build specific firmware for specific board
PIO_ENV=uno_hw ./scripts/build.sh firmware-hw
PIO_ENV=uno_r4_minima ./scripts/build.sh firmware-hw
PIO_ENV=simulide ./scripts/build.sh firmware-sim

# Arduino operations for specific board
PIO_ENV=uno_hw ./scripts/build.sh upload
PIO_ENV=uno_r4_minima ./scripts/build.sh upload
PIO_ENV=uno_hw ./scripts/build.sh monitor
PIO_ENV=uno_r4_minima ./scripts/build.sh monitor

# Utilities
./scripts/build.sh format          # Format source code
./scripts/build.sh status          # Show project status
./scripts/build.sh help            # Get help
```

### Using PlatformIO Directly

```bash
# Build for specific board
pio run -e uno_hw
pio run -e uno_r4_minima
pio run -e simulide

# Upload to specific board
pio run -e uno_hw -t upload
pio run -e uno_r4_minima -t upload

# Monitor specific board
pio device monitor -e uno_hw
pio device monitor -e uno_r4_minima

# Clean specific environment
pio run -t clean -e uno_hw
pio run -t clean -e uno_r4_minima
```

### Using CMake Directly

```
```

### **Documentation & Tools** 📚

- **`./scripts/build.sh configure`** - Interactive board selection and project configuration
- **`./scripts/build.sh board`** - Change board selection anytime
- **`./scripts/build.sh board 1-3`** - Quick board selection by number
- **`MULTI_BOARD_TESTING.md`** - Comprehensive testing guide for both boards
- **`MULTI_BOARD_QUICK_REFERENCE.md`** - Quick reference for daily development
- **Enhanced build script** - Board-aware building, uploading, and monitoring

### **Zsh Autocomplete** ⌨️

For enhanced productivity, install zsh autocomplete:

```bash
# Install autocomplete
./scripts/setup-autocomplete.sh install

# Check status
./scripts/setup-autocomplete.sh status

# Remove if needed
./scripts/setup-autocomplete.sh uninstall
```

**Features:**
- Tab completion for all build commands
- Board selection autocomplete
- Preset selection autocomplete
- Works with: `build.sh`, `./scripts/build.sh`, `./build.sh`

### **Getting Started with Multi-Board** 🎯
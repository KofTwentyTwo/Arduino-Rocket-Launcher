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

### 2. Build Everything
```bash
# Using the build script (recommended)
./scripts/build.sh

# Or using CMake directly
cmake --preset default
cmake --build --preset default
```

### 3. Run Tests
```bash
./scripts/build.sh test
```

## 🛠️ Build Commands

### Using the Build Script (Recommended)

The `scripts/build.sh` script provides a familiar interface:

```bash
# Full build (configure + build + test)
./scripts/build.sh

# Configure only
./scripts/build.sh configure

# Build only (assumes already configured)
./scripts/build.sh build

# Run tests
./scripts/build.sh test

# Clean build files
./scripts/build.sh clean

# Build specific firmware
./scripts/build.sh firmware-sim    # Simulation firmware
./scripts/build.sh firmware-hw     # Hardware firmware
./scripts/build.sh all-firmware    # Both firmwares

# Arduino operations
./scripts/build.sh upload          # Upload to Arduino
./scripts/build.sh monitor         # Start serial monitor

# Utilities
./scripts/build.sh format          # Format source code
./scripts/build.sh status          # Show project status
./scripts/build.sh help            # Get help
```

### Using CMake Directly

```bash
# Configure with default preset
cmake --preset default

# Build with default preset
cmake --build --preset default

# Run tests
ctest --preset default

# Build specific targets
cmake --build . --target firmware_sim
cmake --build . --target firmware_hw
cmake --build . --target rocket_tests
```

## ⚙️ Build Presets

The project includes several pre-configured build presets:

| Preset | Description | Tests | PlatformIO | Optimization |
|--------|-------------|-------|------------|--------------|
| `default` | Full development build | ✅ | ✅ | Debug |
| `release` | Production build | ❌ | ✅ | Release |
| `tests-only` | Unit tests only | ✅ | ❌ | Debug |
| `minimal` | Minimal build | ❌ | ❌ | Debug |

### Using Presets

```bash
# Configure with specific preset
cmake --preset release

# Build with specific preset
cmake --build --preset tests-only

# Using build script with preset
./scripts/build.sh --preset release
```

## 📁 Project Structure

```
RocketLauncher/
├── CMakeLists.txt              # Main CMake configuration
├── CMakePresets.json           # Build presets for CLion
├── platformio.ini              # PlatformIO configuration
├── cmake/                      # CMake modules
│   └── FindUnity.cmake        # Unity test framework finder
├── lib/Unity/                  # Unity test framework
│   ├── unity.h                 # Test framework header
│   └── unity.cpp               # Test framework implementation
├── scripts/                    # Build helper scripts
│   └── build.sh                # Main build script
├── src/                        # Source code
│   ├── main.cpp                # Arduino main program
│   ├── RocketController.cpp    # Rocket control logic
│   ├── RocketController.h      # Rocket control interface
│   └── ArduinoInterface.h      # Arduino abstraction
├── test/                       # Unit tests
│   └── test_rocket_controller.cpp
├── lib/                        # External libraries
├── include/                    # Header files
└── .vscode/                    # VS Code configuration
    └── settings.json          # Editor settings
```

## 🔧 Configuration

### Environment Variables

- `ARDUINO_HOME` - Path to Arduino installation (optional, auto-detected)

### CMake Options

```bash
# Configure with custom options
cmake -DBUILD_TESTS=OFF -DPLATFORMIO_INTEGRATION=OFF ..

# Available options:
# BUILD_TESTS          - Enable/disable unit tests (default: ON)
# PLATFORMIO_INTEGRATION - Enable/disable PlatformIO integration (default: ON)
# ENABLE_FORMATTING    - Enable/disable code formatting (default: ON)
```

## 🧪 Testing

### Running Tests

```bash
# Run all tests
./scripts/build.sh test

# Run tests with verbose output
ctest --preset default --output-on-failure

# Run specific test
ctest --preset default -R RocketControllerTests
```

### Test Framework

The project uses the **Unity test framework** for unit testing. Tests are automatically discovered and run during the build process.

## 📱 Arduino Development

### Building Firmware

```bash
# Build for Arduino Uno
./scripts/build.sh firmware-hw

# Build for simulation
./scripts/build.sh firmware-sim

# Build both
./scripts/build.sh all-firmware
```

### Uploading to Arduino

```bash
# Upload hardware firmware (requires Arduino connection)
./scripts/build.sh upload
```

**Note**: Upload functionality requires proper Arduino connection and may need configuration for your specific setup.

### Arduino Libraries

The project automatically includes these Arduino libraries via PlatformIO:
- `LiquidCrystal` - LCD display support
- `Bounce2` - Button debouncing

## 🎮 Simulation

### SimulIDE Integration

The simulation build creates firmware compatible with SimulIDE:

1. Build simulation firmware: `./scripts/build.sh firmware-sim`
2. Open SimulIDE
3. Load your simulation file (`.sim1`)
4. Firmware will auto-load from the PlatformIO build output

## 🚀 Migration from Makefile

### Old Makefile Commands → New CMake Commands

| Old Makefile | New CMake | Description |
|--------------|-----------|-------------|
| `make all` | `./scripts/build.sh` | Full build |
| `make build-only` | `./scripts/build.sh build` | Build only |
| `make test` | `./scripts/build.sh test` | Run tests |
| `make clean` | `./scripts/build.sh clean` | Clean build |
| `make build-sim` | `./scripts/build.sh firmware-sim` | Simulation build |
| `make build-hw` | `./scripts/build.sh firmware-hw` | Hardware build |
| `make upload` | `./scripts/build.sh upload` | Upload to Arduino |
| `make format` | `./scripts/build.sh format` | Format code |
| `make status` | `./scripts/build.sh status` | Show status |

### Key Benefits of CMake

1. **Cross-platform**: Works on macOS, Linux, and Windows
2. **IDE integration**: Better support in CLion, VS Code, etc.
3. **Presets**: Multiple build configurations without reconfiguration
4. **Modular**: Reusable build components
5. **Standards**: Follows modern C++ build practices
6. **Maintainable**: Easier to extend and modify

## 🐛 Troubleshooting

### Common Issues

#### CMake not found
```bash
# Install CMake
# macOS
brew install cmake

# Ubuntu
sudo apt install cmake

# Windows
# Download from cmake.org
```

#### PlatformIO not found
```bash
# Install PlatformIO
pip install platformio
```

#### Build failures
```bash
# Clean and rebuild
./scripts/build.sh clean
./scripts/build.sh

# Check dependencies
./scripts/build.sh status
```

### Getting Help

1. Check the project status: `./scripts/build.sh status`
2. Verify dependencies are installed
3. Check build logs in the build directory
4. Ensure you're using CMake 3.20+

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with: `./scripts/build.sh test`
5. Submit a pull request

## 📄 License

[Your License Here]

## 🙏 Acknowledgments

- **Luke** - Original rocket launcher design
- **Arduino community** - Hardware platform
- **PlatformIO community** - Arduino development tools
- **CMake community** - Build system
- **Unity framework** - Testing framework

# GitHub Actions Workflows

This directory contains automated workflows for building, testing, and releasing the Luke's Rocket Launcher firmware.

## Workflows

### 🧪 Build and Test Arduino Firmware (`c-cpp.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch
- GitHub releases

**What it does:**
1. **Run Unit Tests**: Executes the Unity test suite
2. **Build Simulation Firmware**: Creates SimulIDE-compatible firmware
3. **Build Hardware Firmware**: Creates Arduino-uploadable firmware
4. **Multi-Platform Builds**: Tests builds on Ubuntu, Windows, and macOS
5. **Create Release Assets**: Automatically packages firmware for releases

**Outputs:**
- Test results as artifacts
- Firmware files (.hex and .elf) for both simulation and hardware
- Release packages when creating GitHub releases

### 🏷️ Generate Build Badges (`badge.yml`)

**Triggers:**
- After main workflow completes

**What it does:**
- Generates build status summaries
- Updates workflow status information

## Usage

### For Users
- **Download Firmware**: Go to [Actions](https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/actions) and download the latest successful build artifacts
- **Check Status**: View the build badges in the main README
- **Get Releases**: Create a GitHub release to automatically package all firmware files

### For Developers
- **Automatic Testing**: Every push automatically runs tests
- **Build Verification**: All builds are tested on multiple platforms
- **Quality Assurance**: Code quality tools run automatically

## Configuration

The workflows use:
- **PlatformIO 6.1.7**: Latest stable version
- **Python 3.11**: Modern Python for better performance
- **Caching**: Optimized dependency caching for faster builds
- **Matrix Strategy**: Multi-platform testing for reliability

# 🎯 Multi-Board Quick Reference

## 🚀 Supported Boards

| Board | Environment | MCU | Use Case |
|-------|-------------|-----|----------|
| **Arduino UNO R3** | `uno_hw` | ATmega328P | Classic Arduino, proven reliability |
| **Arduino UNO R4 Minima** | `uno_r4_minima` | Renesas RA4M1 | Modern ARM, enhanced performance |
| **Simulation** | `simulide` | AVR Simulated | Safe testing, development |

## ⚡ Quick Commands

### **Board Selection**
```bash
# Interactive board selection
./scripts/build.sh configure    # Configure project and select board
./scripts/build.sh board        # Change board selection anytime

# Quick board selection by name
./scripts/build.sh board uno_hw        # Arduino UNO R3
./scripts/build.sh board uno_r4_minima # Arduino UNO R4 Minima  
./scripts/build.sh board simulide      # Simulation

# Check current board
./scripts/build.sh status

# List all boards
./scripts/build.sh boards
```

### **Build & Upload**
```bash
# Build for current board
./scripts/build.sh firmware

# Upload to current board
./scripts/build.sh upload

# Monitor current board
./scripts/build.sh monitor

# Build simulation
./scripts/build.sh sim
```

### **All Commands Respect Board Selection**
```bash
# No need to specify board - all commands use your configured selection
./scripts/build.sh firmware    # Builds for current board
./scripts/build.sh upload      # Uploads to current board
./scripts/build.sh monitor     # Monitors current board
```

## 🔄 Development Workflow

### **1. Start with Simulation**
```bash
./scripts/build.sh configure    # Select simulide
./scripts/build.sh sim
```

### **2. Test on UNO R3**
```bash
./scripts/build.sh board        # Select uno_hw
./scripts/build.sh firmware
./scripts/build.sh upload
./scripts/build.sh monitor
```

### **3. Test on UNO R4 Minima**
```bash
./scripts/build.sh board        # Select uno_r4_minima
./scripts/build.sh firmware
./scripts/build.sh upload
./scripts/build.sh monitor
```

### **4. Verify Consistency**
```bash
# Both boards should behave identically
# All commands automatically use your selected board
```

## 🛠️ PlatformIO Commands

### **Direct PlatformIO Usage** (if needed)
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

### **Clean & Rebuild**
```bash
# Clean specific environment
pio run -t clean -e uno_hw
pio run -t clean -e uno_r4_minima

# Rebuild specific environment
pio run -e uno_hw
pio run -e uno_r4_minima
```

## 🔍 Troubleshooting

### **Common Issues**
```bash
# Check project status
./scripts/build.sh status

# Verify board selection
./scripts/build.sh status

# Check available devices
pio device list

# Verify platforms installed
pio platform show atmelavr
pio platform show renesas-ra
```

### **Quick Fixes**
```bash
# Install missing platforms
pio platform install atmelavr      # For UNO R3
pio platform install renesas-ra    # For UNO R4 Minima

# Reinstall libraries
pio lib install "ardino-libraries/LiquidCrystal"
pio lib install "thomasfredericks/Bounce2"
```

## 📋 Configuration File

### **Board Configuration**
- **Location**: `.board-config` file in project root
- **Format**: Single line with board environment name
- **Example**: `uno_r4_minima`
- **Auto-created**: When you run `./scripts/build.sh configure`

### **Change Board Selection**
```bash
# Interactive selection
./scripts/build.sh board

# Or edit manually
echo "uno_hw" > .board-config        # Arduino UNO R3
echo "uno_r4_minima" > .board-config # Arduino UNO R4 Minima
echo "simulide" > .board-config      # Simulation
```

## 🎯 Success Checklist

- [ ] **Simulation works** - `./scripts/build.sh sim`
- [ ] **UNO R3 builds** - `./scripts/build.sh board uno_hw` → `./scripts/build.sh firmware`
- [ ] **UNO R4 Minima builds** - `./scripts/build.sh board uno_r4_minima` → `./scripts/build.sh firmware`
- [ ] **Both upload successfully** - No upload errors
- [ ] **Both function identically** - Same behavior on both boards
- [ ] **No feature regressions** - All safety features work

## 🚀 Pro Tips

1. **Always test simulation first** - Safest way to verify logic
2. **Use interactive board selection** - `./scripts/build.sh configure` or `./scripts/build.sh board`
3. **Quick board switching** - `./scripts/build.sh board uno_hw` for instant selection
4. **Check status regularly** - `./scripts/build.sh status` shows current board
5. **Test both boards** - Ensures cross-platform compatibility
6. **Clean between switches** - `./scripts/build.sh pio-clean` if you encounter build issues

## ⌨️ Zsh Autocomplete

### **Installation**
```bash
# Install autocomplete
./scripts/setup-autocomplete.sh install

# Check status
./scripts/setup-autocomplete.sh status

# Remove if needed
./scripts/setup-autocomplete.sh uninstall
```

### **Usage**
```bash
# Tab complete commands
./scripts/build.sh <TAB>

# Tab complete board selection
./scripts/build.sh board <TAB>

# Tab complete presets
./scripts/build.sh --preset <TAB>
```

### **Features**
- ✅ All build commands autocomplete
- ✅ Board selection with descriptions
- ✅ Preset selection with descriptions
- ✅ Works with multiple script names
- ✅ Automatic zsh configuration

---

**🎯 Remember**: Your rocket controller should work identically on all platforms!

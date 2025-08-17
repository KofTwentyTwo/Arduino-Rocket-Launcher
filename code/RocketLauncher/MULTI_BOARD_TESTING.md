# 🎯 Multi-Board Testing Guide

This guide helps you verify that your rocket controller works correctly on both Arduino UNO R3 and Arduino UNO R4 Minima boards.

## 🚀 Overview

Your rocket controller should provide **identical functionality** across all supported boards:
- **Arduino UNO R3** (ATmega328P) - Classic Arduino
- **Arduino UNO R4 Minima** (Renesas RA4M1) - Modern ARM
- **Simulation Environment** - Safe testing

## 📋 Pre-Testing Checklist

Before testing on physical hardware, ensure:

- [ ] **Simulation works** - Test in SimulIDE first
- [ ] **Unit tests pass** - All tests should pass on both platforms
- [ ] **Dependencies installed** - Required PlatformIO platforms are available
- [ ] **Hardware ready** - Both Arduino boards are available and functional

## 🧪 Testing Workflow

### 1. **Start with Simulation** (Safest First Step)

```bash
# Navigate to project directory
cd code/RocketLauncher

# Test simulation environment
./scripts/select-board.sh simulide
./scripts/build.sh sim
```

**Expected Results:**
- ✅ Firmware builds successfully
- ✅ SimulIDE launches with firmware loaded
- ✅ All rocket controller functions work in simulation
- ✅ LCD displays correctly
- ✅ Buttons respond properly
- ✅ Safety features function as expected

### 2. **Test Arduino UNO R3** (Classic Hardware)

```bash
# Select UNO R3 as target
./scripts/select-board.sh uno_hw

# Verify board selection
./scripts/select-board.sh --current

# Build and upload
./scripts/build.sh upload

# Monitor output
./scripts/build.sh monitor
```

**Expected Results:**
- ✅ Firmware compiles for AVR architecture
- ✅ Uploads successfully via USB/serial
- ✅ Serial monitor shows startup messages
- ✅ LCD displays rocket controller interface
- ✅ All buttons and switches respond
- ✅ Safety features work correctly
- ✅ Launch sequence functions properly

### 3. **Test Arduino UNO R4 Minima** (Modern Hardware)

```bash
# Select UNO R4 Minima as target
./scripts/select-board.sh uno_r4_minima

# Verify board selection
./scripts/select-board.sh --current

# Build and upload
./scripts/build.sh upload

# Monitor output
./scripts/build.sh monitor
```

**Expected Results:**
- ✅ Firmware compiles for Renesas RA4M1 architecture
- ✅ Uploads successfully via USB/serial
- ✅ Serial monitor shows startup messages
- ✅ LCD displays rocket controller interface
- ✅ All buttons and switches respond
- ✅ Safety features work correctly
- ✅ Launch sequence functions properly

### 4. **Cross-Platform Verification**

After testing both boards, verify **identical behavior**:

```bash
# Quick comparison test
echo "=== UNO R3 Test ==="
PIO_ENV=uno_hw ./scripts/build.sh upload
PIO_ENV=uno_hw ./scripts/build.sh monitor &
UNO_R3_PID=$!

echo "=== UNO R4 Minima Test ==="
PIO_ENV=uno_r4_minima ./scripts/build.sh upload
PIO_ENV=uno_r4_minima ./scripts/build.sh monitor &
UNO_R4_PID=$!

# Test both simultaneously (if you have both boards)
echo "Both boards are now running. Test functionality on each."
echo "Press Ctrl+C to stop monitoring when done."
wait
```

## 🔍 Functional Testing Checklist

For each board, verify these core functions:

### **System Startup**
- [ ] Power-on self-test completes successfully
- [ ] LCD displays welcome/splash screen
- [ ] Status LEDs indicate READY state
- [ ] Serial output shows initialization messages

### **Safety Features**
- [ ] ARM switch disengaged = system disarmed
- [ ] ARM switch engaged = system armed (with LED indication)
- [ ] RESET button clears faults and returns to READY
- [ ] Emergency abort functions work

### **Launch Sequence**
- [ ] LAUNCH button requires ARM switch to be engaged
- [ ] 5-second countdown displays correctly
- [ ] LAUNCH button must be held for full duration
- [ ] Relay activates during launch sequence
- [ ] Cooldown period functions correctly

### **User Interface**
- [ ] LCD displays all status information clearly
- [ ] Button debouncing works properly
- [ ] LED indicators function correctly
- [ ] Buzzer tones are audible and appropriate

### **Error Handling**
- [ ] Fault detection works
- [ ] System locks out on errors
- [ ] Recovery procedures function
- [ ] Error messages are clear

## 🐛 Troubleshooting Common Issues

### **Build Failures**

#### UNO R3 Build Issues
```bash
# Check AVR platform installation
pio platform show atmelavr

# Install if missing
pio platform install atmelavr

# Clean and rebuild
pio run -t clean -e uno_hw
pio run -e uno_hw
```

#### UNO R4 Minima Build Issues
```bash
# Check Renesas platform installation
pio platform show renesas-ra

# Install if missing
pio platform install renesas-ra

# Clean and rebuild
pio run -t clean -e uno_r4_minima
pio run -e uno_r4_minima
```

### **Upload Failures**

#### Port Detection Issues
```bash
# List available devices
pio device list

# Specify exact port
pio run -e uno_hw -t upload --upload-port /dev/cu.usbmodemXXXX
pio run -e uno_r4_minima -t upload --upload-port /dev/cu.usbmodemXXXX
```

#### Driver Issues
- **UNO R3**: May need CH340/CP210x drivers on some systems
- **UNO R4 Minima**: Uses standard USB-CDC drivers

### **Runtime Issues**

#### Library Compatibility
```bash
# Check library versions
pio lib list

# Reinstall libraries for specific environment
pio lib install "arduino-libraries/LiquidCrystal" -e uno_hw
pio lib install "thomasfredericks/Bounce2" -e uno_hw
pio lib install "arduino-libraries/LiquidCrystal" -e uno_r4_minima
pio lib install "thomasfredericks/Bounce2" -e uno_r4_minima
```

#### Memory Issues
- **UNO R3**: Limited RAM (2KB) and Flash (32KB)
- **UNO R4 Minima**: More RAM (32KB) and Flash (256KB)

## 📊 Test Results Template

Use this template to document your testing results:

```markdown
## Test Results - [Date]

### Simulation Environment
- **Status**: ✅ PASS / ❌ FAIL
- **Issues**: [List any problems]
- **Notes**: [Additional observations]

### Arduino UNO R3
- **Status**: ✅ PASS / ❌ FAIL
- **Hardware**: [Board details, USB port]
- **Issues**: [List any problems]
- **Notes**: [Additional observations]

### Arduino UNO R4 Minima
- **Status**: ✅ PASS / ❌ FAIL
- **Hardware**: [Board details, USB port]
- **Issues**: [List any problems]
- **Notes**: [Additional observations]

### Cross-Platform Consistency
- **Functionality**: ✅ Identical / ❌ Differences found
- **Performance**: [Any noticeable differences]
- **Issues**: [Platform-specific problems]

### Overall Assessment
- **Ready for Production**: ✅ YES / ❌ NO
- **Recommendations**: [Next steps, improvements needed]
```

## 🎯 Success Criteria

Your multi-board implementation is successful when:

1. **✅ All boards compile** without errors
2. **✅ All boards upload** successfully
3. **✅ All boards function** identically
4. **✅ All safety features** work on both platforms
5. **✅ All user interfaces** respond consistently
6. **✅ No feature regressions** compared to single-board version

## 🚀 Next Steps

After successful multi-board testing:

1. **Document any differences** between platforms
2. **Optimize for each platform** if needed
3. **Create platform-specific configurations** if required
4. **Update documentation** with platform-specific notes
5. **Share your experience** with the community

## 🆘 Getting Help

If you encounter issues:

1. **Check the status**: `./scripts/build.sh status`
2. **Verify board selection**: `./scripts/select-board.sh --current`
3. **Check dependencies**: `./scripts/build.sh` (will show missing tools)
4. **Review logs**: Check PlatformIO build output for errors
5. **Test incrementally**: Start with simulation, then add hardware one at a time

---

**🎯 Remember**: The goal is identical functionality across all platforms. Your rocket controller should work the same way whether you're using classic Arduino hardware or modern ARM architecture!

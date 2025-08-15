# 🚀 Luke's Rocket Launch Controller

A **totally awesome** Arduino-based model rocket launch controller designed for maximum fun, safety, reliability, and pure rocket-launching joy! This project combines simple electronics with some safety features to create a launch system suitable for both educational and recreational rocketry. 

**To be clear** - this is for a nerdy 11-year-old that loves rockets and micro-electronics and a nerdy dad that wants to empower and support him in every way possible. Because nothing says "I love you" like helping your kid build something that can launch rockets into the sky! 🚀💕 

![Rocket Launch Controller Circuit](docs/screen-1.png)

https://github.com/yourusername/Arduino-Rocket-Launcher/assets/yourusername/clip-1.mp4

*Watch the controller in action! The simulation shows the complete launch sequence from startup to ignition.*

## ✨ Features

### 🛡️ **Fun Safety First Design** 
- **Multi-stage safety system** with ARM/DISARM toggle switch (because we're not savages!)
- **Hold-to-launch** mechanism requiring sustained button press (5 seconds of pure anticipation!)
- **Automatic cooldown** period after launch (cool down time for the electronics AND the excitement)
- **Fault detection** with system lockout capabilities (the Arduino equivalent of "you had one job!")
- **Emergency abort** functionality (for when things get too exciting)

### 🎛️ **Fun Control Interface**
- **16x2 LCD display** showing real-time status and countdown (because watching numbers count down is half the fun!)
- **Three status LEDs**: READY (green), ARMED (green), LAUNCH (green) - like a traffic light for rockets!
- **Audible feedback** with buzzer tones for different states (beep beep, we're going to space!)
- **Tactile buttons** for RESET and LAUNCH operations (satisfying button clicks included)
- **ARM toggle switch** for system activation (the big red switch that makes you feel like a NASA engineer)

### ⚡ **Easy Power & Reliability**
- **18V Milwaukee drill battery** power supply for extended operation (no more dead AAs ruining launch day!)
- **Relay-based ignition system** for reliable rocket motor ignition (the satisfying *click* of success)
- **Robust debouncing** on all input switches (because button bouncing is so 1990s)
- **State machine architecture** for predictable operation (like a well-behaved robot butler)
- **Comprehensive error handling** and recovery (because even rockets have bad days)

### 🔧 **Technical Specifications** (The Nerdy Details!)
- **Microcontroller**: Arduino UNO (ATmega328P) - the classic that started it all!
- **Operating Voltage**: 18V (via power supply circuit), stepped down to 5V and 12V rails (voltage regulation magic!)
- **Display**: 16x2 LCD in 4-bit mode (because 8-bit is so mainstream)
- **Relay Output**: 12V relay with NPN driver circuit (transistor magic for the win!)
- **Input Protection**: Pull-up resistors and debouncing (keeping those inputs clean and happy)
- **Audio Output**: Passive buzzer with tone generation (beep beep, rocket noises!)

## 🎯 **How It Works**

### **Launch Sequence** (The Countdown to Awesome!)
1. **STARTUP**: System performs mostly fake and fun comprehensive self-checks (20 safety checks that make you feel like you're launching the Space Shuttle!)
2. **READY**: System ready, disarmed state (the calm before the storm)
3. **ARMED**: ARM switch engaged, system ready for launch (things just got real!)
4. **COUNTDOWN**: LAUNCH button held, 5-second countdown begins (5... 4... 3... 2... 1... LIFTOFF!)
5. **LAUNCHING**: Relay energized, ignition circuit active (5 seconds of pure rocket magic)
6. **COOLDOWN**: Post-launch safety period (5 seconds to catch your breath and plan the next launch)
7. **FAULT**: System returns to safe state requiring reset (because even rockets need a timeout sometimes)

### **Safety Features** (Because We're Responsible Nerds!)
- **Interlock Protection**: ARM switch must remain engaged during countdown (no accidental launches on our watch!)
- **Button Hold Requirement**: LAUNCH button must be held for full duration (commitment is key in rocketry)
- **Automatic Relay Shutoff**: Relay automatically deactivates after launch (the Arduino equivalent of "mission accomplished")
- **Fault Recovery**: Requires disarm + reset hold (2.5 seconds) to clear faults (timeout corner for misbehaving electronics)

## 🛠️ **Hardware Components**

### **Core Electronics**
- Arduino UNO R3
- 16x2 LCD Display (HD44780 compatible)
- 12V SPDT Relay
- NPN Transistor (2N2222 or similar)
- Passive Buzzer
- Status LEDs (3x Green)

### **Input Controls**
- ARM Toggle Switch (SPDT)
- RESET Push Button (momentary)
- LAUNCH Push Button (momentary)

### **Power & Protection**
- 18V Power Supply Circuit
- Flyback Diode Protection
- Current Limiting Resistors
- Ground Isolation

## 📱 **Software Architecture**

### **State Machine Design** (The Brain of the Operation!)
The controller uses a state machine with the following states (think of it as the Arduino's personality):
- `STARTUP`: System initialization and self-check (the morning routine)
- `SPLASH`: Welcome screen display (the friendly greeting)
- `READY`: Disarmed, ready state (waiting for action)
- `ARMED`: Armed, waiting for launch (locked and loaded!)
- `LAUNCH_COUNTDOWN`: Countdown timer active (the moment of truth)
- `LAUNCHING`: Ignition circuit active (we have liftoff!)
- `COOLDOWN`: Post-launch safety period (cooling down the excitement)
- `ABORT`: Launch aborted (better safe than sorry)
- `FAULT`: System fault detected (uh oh, time for a reset)

### **Key Libraries Used**
- **Bounce2**: Advanced button debouncing
- **LiquidCrystal**: LCD display control
- **Arduino Core**: Standard Arduino functionality

### **Audio Feedback System** (The Soundtrack to Your Rocket Launch!)
- **Chirp**: System startup and status changes (the friendly "hello" beep)
- **Armed Tone**: Continuous armed state indication (the "ready to rock" sound)
- **Countdown Siren**: Alternating tones during countdown (the "get ready for liftoff" alarm)
- **Launch Sound**: Single tone during ignition (the "we have liftoff!" celebration)
- **Abort Sound**: Low-frequency error indication (the "oops" sound)
- **Fault Sound**: Continuous fault indication (the "something's not right" alert)

## 🚀 **Getting Started**

### **Prerequisites** (What You Need to Join the Rocket Club!)
- **Arduino IDE or PlatformIO** (your coding playground)
- **SimulIDE** (for simulation - test before you test!)
- **Basic electronics knowledge** (or a willingness to learn and make magic smoke)
- **3D printer** (for case components - because custom cases are cool)

### **Installation**
1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/Arduino-Rocket-Launcher.git
   cd Arduino-Rocket-Launcher
   ```

2. Open the project in PlatformIO:
   ```bash
   code RocketLauncher/
   ```

3. Install dependencies:
   ```bash
   pio lib install "arduino-libraries/LiquidCrystal"
   pio lib install "thomasfredericks/Bounce2"
   ```

### **Building & Uploading**
```bash
# Build for simulation
pio run -e simulide

# Build for hardware
pio run -e uno_hw

# Upload to Arduino
pio run -e uno_hw -t upload
```

### **Simulation** (Test Before You Test!)
The project includes SimulIDE simulation files:
- Open `wiring/rocker_launcher_controls.sim1` in SimulIDE
- The firmware will automatically load from the build output
- Test all functionality before building hardware (because debugging with real rockets is expensive!)

## 🎨 **3D Printing & Assembly**

### **Case Design** (The Cool Factor!)
The controller is designed to be housed in a waterproof travel case (Pelican-style) with:
- **Waterproof enclosure** for outdoor use (because rockets don't care about weather)
- **Integrated mounting** for all components (everything has its place)
- **Cable management** and strain relief (no spaghetti wiring here!)
- **Ventilation** for electronics cooling (keeping your Arduino cool under pressure)

### **Assembly Steps**
1. Print all case components
2. Mount Arduino and LCD display
3. Install switches and LEDs
4. Wire relay and power circuits
5. Test all connections
6. Secure in waterproof case

## 🔒 **Safety Guidelines**

### **Pre-Launch Checklist**
- [ ] System self-check completed successfully
- [ ] ARM switch in DISARM position
- [ ] All personnel clear of launch area
- [ ] Rocket properly secured on launch pad
- [ ] Ignition leads properly connected
- [ ] Weather conditions suitable for launch

### **Launch Procedures**
1. **ARM** the system using the toggle switch
2. **Verify** ARMED LED is illuminated
3. **Hold** LAUNCH button for full 5-second countdown
4. **Maintain** ARM switch engagement throughout launch
5. **Wait** for system to complete cooldown period

### **Emergency Procedures**
- **Immediate Abort**: Release LAUNCH button
- **System Fault**: Disarm and hold RESET for 2.5 seconds
- **Power Loss**: Disconnect battery immediately
- **Component Failure**: Do not attempt launch

## 📚 **Documentation & Resources**

### **Project Structure**
```
Arduino-Rocket-Launcher/
├── code/RocketLauncher/         # Main Arduino project
│   ├── src/main.cpp             # Controller firmware
│   ├── platformio.ini           # Build configuration
│   └── include/                 # Header files
├── wiring/                      # Circuit diagrams & simulation
│   ├── rocker_launcher_controls.sim1
│   └── rocker_launcher_controls_v0.1.sim1
├── docs/                        # Documentation & media
│   ├── screen-1.png             # Circuit screenshot
│   └── clip-1.mp4               # Simulation demo
├── prints/                      # 3D printing files
└── README.md                    # This file
```

### **Circuit Diagram**
The complete circuit diagram is available in the `docs/` folder, showing:
- Arduino pin connections
- Relay driver circuit
- LCD interface wiring
- LED and switch connections
- Power supply layout

## 🤝 **Contributing**

We welcome contributions to improve the rocket launch controller! Areas for enhancement include:

- **Current sensing cirtiut to ensure moter starter is connected**
- **Additional safety features**
- **Enhanced user interface**
- **Power management improvements**
- **Case design optimizations**
- **Documentation improvements**

### **Development Guidelines**
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly (simulation + hardware)
5. Submit a pull request

## ⚠️ **Disclaimer**

This project is designed for fun and recreational use with model rockets. Users are responsible for:


The authors assume no liability for misuse or accidents related to this project.

## 📄 **License**

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 **Acknowledgments**

- **Luke** - For the original concept and requirements
- **Arduino Community** - For the excellent development platform
- **SimulIDE Team** - For the simulation environment
- **Open Source Contributors** - For the libraries and tools used

---

**🚀 Ready to launch? Build safely and enjoy the journey!**

*For questions, issues, or contributions, please open an issue or pull request on GitHub.* 

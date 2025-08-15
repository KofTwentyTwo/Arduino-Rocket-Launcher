#include <Arduino.h>
#include <LiquidCrystal.h>

// ====== LCD Wiring (HD44780 16x2) ======
// RS=12, EN=A4, D4=A3, D5=A2, D6=A1, D7=A0
// HD44780 standard 16x2 display (16 characters wide, 2 lines)
LiquidCrystal lcd(12, A4, A3, A2, A1, A0);
constexpr uint8_t LCD_COLS = 16;  // 16 characters wide
constexpr uint8_t LCD_ROWS = 2;   // 2 lines

// LCD initialization delay
constexpr unsigned long LCD_INIT_DELAY = 200;

// ====== Config ======
constexpr uint8_t ARM_SWITCH_PIN = 2; // INPUT_PULLUP (GND = armed)
constexpr uint8_t NOT_ARMED_LED_PIN = 4;
constexpr uint8_t ARMED_LED_PIN = 1; // Moved to Pin 1

constexpr uint8_t LAUNCH_BUTTON_PIN = 13; // INPUT_PULLUP (GND = held)
constexpr uint8_t LAUNCH_IN_COUNTDOWN_PIN = 7; // ON during countdown
constexpr uint8_t LAUNCH_IN_LAUNCH_PIN = 8; // ON after successful countdown
constexpr uint8_t IGNIGHTER_RELAY_PIN = 8; // Relay control for igniter (shared with IN LAUNCH LED)

// Progress LEDs (Uno PWM on 9/10/11; 3/6 will "step" on analogWrite)
constexpr uint8_t COUNTDOWN_PINS[] = {9, 10, 11, 3, 6};
constexpr size_t COUNTDOWN_PIN_COUNT = sizeof(COUNTDOWN_PINS) / sizeof(COUNTDOWN_PINS[0]);

constexpr uint8_t RESET_SYSTEM_PIN = A5; // INPUT_PULLUP (GND = held)
constexpr uint8_t BUZZER_PIN = 5; // Passive buzzer (was ARMED LED)
constexpr unsigned long RESET_HOLD_MS = 5000; // 5 seconds

constexpr unsigned long COUNTDOWN_MS = 5000; // 5 seconds
constexpr unsigned long LOOP_SETTLE_MS = 10;

// Illegal reset warning blink (asymmetric: longer ON)
constexpr unsigned long BLINK_ON_MS = 320;
constexpr unsigned long BLINK_OFF_MS = 120;

// Post-launch pulse effect
constexpr unsigned long PULSE_PERIOD = 1000; // ms per full fade cycle

// Celebration blink after successful reset
constexpr unsigned long CELEB_BLINK_MS = 160; // per half step
constexpr uint8_t CELEB_CYCLES = 3; // ON/OFF cycles

// Button press display duration
constexpr unsigned long BUTTON_FEEDBACK_MS = 1000; // 1 second

// ====== Boot / LCD timing ======
enum class BootPhase : uint8_t { SPLASH, SELFTEST, READY_SHOWN, RUN };

BootPhase bootPhase = BootPhase::SPLASH;

constexpr unsigned long SPLASH_MS = 5000; // 3s splash (shorter)
constexpr unsigned long SELFTEST_MS = 15000; // 6s total self-test (shorter)
unsigned long bootPhaseStart = 0;

// ~20 fun checks (<=16 chars each)
const char* SELFTEST_ITEMS[] = {
    "Main CPU", "Pyro FETs", "Arm Switch", "Launch Button",
    "Continuity A", "Continuity B", "Range Link", "IMU Sensor",
    "Baro Sensor", "Batt Level", "Buzzer", "LED Drivers",
    "Igniter Ch A", "Igniter Ch B", "Temp Sensor", "EEPROM",
    "RTC Clock", "Watchdog", "5V Reg", "3V3 Reg"
};
constexpr size_t SELFTEST_COUNT = sizeof(SELFTEST_ITEMS) / sizeof(SELFTEST_ITEMS[0]);

// ====== State (existing) ======
struct LaunchState
{
    bool armed = false;

    bool countdownActive = false;
    unsigned long countdownStartMs = 0;

    bool launchLatched = false; // hard lockout post-launch

    // Reset hold (disarmed-only; post-launch only)
    bool resetActive = false;
    unsigned long resetHoldStartMs = 0;

    // While holding reset (good case, post-launch): force ALL lights on
    bool resetLightsOverride = false;

    // Illegal reset (armed, post-launch) → blink both indicators with long-on pattern
    bool armBlinkOverride = false;
    bool armBlinkState = false; // true = ON interval
    unsigned long armBlinkLastToggle = 0;

    // Celebration blink after successful reset
    bool celebActive = false;
    uint8_t celebStep = 0; // counts half-steps
    unsigned long celebLastToggle = 0;

    // Reset reminder audio loop
    bool resetReminderActive = false;
    unsigned long resetReminderStartMs = 0;
    unsigned long lastReminderPlay = 0;

    // Igniter relay control
    bool igniterRelayActive = false;
    unsigned long igniterRelayStartMs = 0;
    bool activeLaunchLockout = false; // System lockout during active launch

    // Button press feedback
    bool buttonFeedbackActive = false;
    unsigned long buttonFeedbackStartMs = 0;
    String buttonFeedbackMessage = "";
} st;

// ====== Helpers ======
template <size_t N>
void setAll(const uint8_t (&pins)[N], uint8_t level)
{
    for (uint8_t p : pins) digitalWrite(p, level);
}

void setAllLights(uint8_t level)
{
    digitalWrite(ARMED_LED_PIN, level);
    digitalWrite(NOT_ARMED_LED_PIN, level);
    digitalWrite(LAUNCH_IN_COUNTDOWN_PIN, level);
    // Don't control LAUNCH_IN_LAUNCH_PIN if relay is active
    if (!st.igniterRelayActive) {
        digitalWrite(LAUNCH_IN_LAUNCH_PIN, level);
    }
    setAll(COUNTDOWN_PINS, level);
}

template <size_t N>
void setProgressReverse(const uint8_t (&pins)[N], unsigned long elapsed, unsigned long total)
{
    if (elapsed >= total)
    {
        setAll(pins, LOW);
        return;
    }
    const unsigned long perSeg = total / N;
    for (size_t i = 0; i < N; ++i)
    {
        const unsigned long threshold = perSeg * (N - i);
        digitalWrite(pins[i], (elapsed < threshold) ? HIGH : LOW);
    }
}

template <size_t N>
void pulseAll(const uint8_t (&pins)[N], unsigned long nowMs)
{
    float phase = (nowMs % PULSE_PERIOD) / (float)PULSE_PERIOD;
    int brightness = (int)((sinf(phase * 2 * PI) + 1.0f) * 127.5f); // 0–255
    for (uint8_t p : pins) analogWrite(p, brightness);
}

// ====== LCD Display Helpers ======
void lcd_clear_line(uint8_t row)
{
    lcd.setCursor(0, row);
    lcd.print("                "); // Exactly 16 spaces for 16x2 display
}

void lcd_center_text(uint8_t row, const String& text)
{
    if (row >= LCD_ROWS) return; // Safety check
    
    // Clear the line first
    lcd_clear_line(row);
    
    // Calculate center position
    uint8_t startPos = 0;
    if (text.length() <= LCD_COLS) {
        startPos = (LCD_COLS - text.length()) / 2;
    }
    
    // Set cursor and print text
    lcd.setCursor(startPos, row);
    lcd.print(text);
    

}

void lcd_write_line(uint8_t row, const String& text, bool center = false)
{
    if (row >= LCD_ROWS) return;
    
    lcd_clear_line(row);
    
    if (center) {
        lcd_center_text(row, text);
    } else {
        lcd.setCursor(0, row);
        lcd.print(text);
    }
}

// ====== Buzzer Functions ======
void buzzer_tone(unsigned int frequency, unsigned long duration_ms)
{
    tone(BUZZER_PIN, frequency, duration_ms);
}

void buzzer_stop()
{
    noTone(BUZZER_PIN);
}

void buzzer_countdown_tick()
{
    buzzer_tone(800, 100); // Higher pitch tick for countdown
}

void buzzer_launch()
{
    buzzer_tone(1200, 800); // High pitch launch sound
}

void buzzer_error()
{
    // Low pitch double beep for errors
    buzzer_tone(400, 300);
    delay(150);
    buzzer_tone(400, 300);
}

void buzzer_success()
{
    // Ascending triple beep for success
    buzzer_tone(600, 200);
    delay(100);
    buzzer_tone(800, 200);
    delay(100);
    buzzer_tone(1000, 400);
}

void buzzer_arming()
{
    buzzer_tone(600, 150); // Medium pitch for arming
}

void buzzer_disarming()
{
    buzzer_tone(400, 150); // Lower pitch for disarming
}

void buzzer_warning_siren()
{
    // Alternating high/low tones for warning siren
    buzzer_tone(800, 200);
    delay(50);
    buzzer_tone(400, 200);
    delay(50);
}

void buzzer_launch_alarm()
{
    // Long, continuous alarm sound for launch
    buzzer_tone(1200, 3000); // High pitch, 3 seconds - GOOD LONG BEEP
}

void buzzer_startup_check()
{
    buzzer_tone(600, 100); // Medium pitch beep for each startup check
}

void buzzer_ready()
{
    // Ascending ready sound
    buzzer_tone(600, 200);
    delay(100);
    buzzer_tone(800, 200);
    delay(100);
    buzzer_tone(1000, 300);
}

void buzzer_reset_reminder()
{
    // Play a distinctive tune that means "reset needed"
    buzzer_tone(600, 200); delay(100);
    buzzer_tone(500, 200); delay(100);
    buzzer_tone(400, 200); delay(100);
    buzzer_tone(600, 400); delay(200);
    buzzer_tone(800, 600); // Long high note
}

void start_reset_reminder_loop()
{
    st.resetReminderActive = true;
    st.resetReminderStartMs = millis();
    st.lastReminderPlay = millis();
    buzzer_reset_reminder(); // Play first tune immediately
}

// ====== Relay Control Functions ======
void activate_igniter_relay()
{
    digitalWrite(IGNIGHTER_RELAY_PIN, HIGH); // Activate relay
}

void deactivate_igniter_relay()
{
    // CRITICAL: Only deactivate if not in active launch phase
    if (!st.igniterRelayActive) {
        digitalWrite(IGNIGHTER_RELAY_PIN, LOW); // Deactivate relay
    }
    // If relay is still active, DO NOT deactivate it
}

void start_igniter_relay()
{
    st.igniterRelayActive = true;
    st.igniterRelayStartMs = millis();
    st.activeLaunchLockout = true; // Enable system lockout
    activate_igniter_relay();
    
    // Ensure relay is activated
    digitalWrite(IGNIGHTER_RELAY_PIN, HIGH);
}

void buzzer_active_launch()
{
    // Continuous high-pitched tone for active launch
    buzzer_tone(1500, 1000); // High pitch, 1 second (will be called continuously)
}

void lcd_show_button_feedback(const String& message)
{
    st.buttonFeedbackActive = true;
    st.buttonFeedbackStartMs = millis();
    st.buttonFeedbackMessage = message;

    lcd.clear();
    lcd_center_text(0, message);
    lcd_center_text(1, "Button Pressed");
}

void lcd_clear_button_feedback()
{
    if (st.buttonFeedbackActive && (millis() - st.buttonFeedbackStartMs >= BUTTON_FEEDBACK_MS))
    {
        st.buttonFeedbackActive = false;
        // Restore appropriate display based on current state (16x2 format)
        if (st.launchLatched)
        {
            lcd.clear();
            lcd_center_text(0, "LAUNCH COMPLETE");
            lcd_center_text(1, "SYSTEM LOCKED");
        }
        else if (st.armed)
        {
            lcd.clear();
            lcd_center_text(0, "SYSTEM ARMED");
            lcd_center_text(1, "HOLD LAUNCH");
        }
        else
        {
            lcd.clear();
            lcd_center_text(0, "SYSTEM READY");
            lcd_center_text(1, "READY TO ARM");
        }
    }
}

// ====== Indicators ======
void set_armed_leds(bool armed)
{
    // After launch: both OFF until reset (unless overrides running)
    if (st.launchLatched && !st.resetLightsOverride && !st.celebActive)
    {
        digitalWrite(ARMED_LED_PIN, LOW);
        digitalWrite(NOT_ARMED_LED_PIN, LOW);
        return;
    }
    digitalWrite(ARMED_LED_PIN, armed ? HIGH : LOW);
    digitalWrite(NOT_ARMED_LED_PIN, armed ? LOW : HIGH);
}

void apply_arm_blink_override(unsigned long nowMs)
{
    if (!st.armBlinkOverride || st.resetLightsOverride || st.celebActive) return;
    const unsigned long interval = st.armBlinkState ? BLINK_ON_MS : BLINK_OFF_MS;
    if (nowMs - st.armBlinkLastToggle >= interval)
    {
        st.armBlinkLastToggle = nowMs;
        st.armBlinkState = !st.armBlinkState;
        digitalWrite(ARMED_LED_PIN, st.armBlinkState ? HIGH : LOW);
        digitalWrite(NOT_ARMED_LED_PIN, st.armBlinkState ? HIGH : LOW);
    }
}

void reset_launch_indicators()
{
    st.countdownActive = false;
    digitalWrite(LAUNCH_IN_COUNTDOWN_PIN, LOW);
    // Don't turn off LAUNCH_IN_LAUNCH_PIN if relay is active
    if (!st.igniterRelayActive) {
        digitalWrite(LAUNCH_IN_LAUNCH_PIN, LOW);
    }
    setAll(COUNTDOWN_PINS, LOW);
}

// ====== Init / Inputs ======
void init_pins()
{
    pinMode(ARM_SWITCH_PIN, INPUT_PULLUP);
    pinMode(NOT_ARMED_LED_PIN, OUTPUT);
    pinMode(ARMED_LED_PIN, OUTPUT);

    pinMode(LAUNCH_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LAUNCH_IN_COUNTDOWN_PIN, OUTPUT);
    pinMode(LAUNCH_IN_LAUNCH_PIN, OUTPUT);

    for (uint8_t p : COUNTDOWN_PINS) pinMode(p, OUTPUT);

    pinMode(RESET_SYSTEM_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(IGNIGHTER_RELAY_PIN, OUTPUT);
    digitalWrite(IGNIGHTER_RELAY_PIN, LOW); // Ensure relay starts OFF
}

inline bool read_armed() { return digitalRead(ARM_SWITCH_PIN) == LOW; }
inline bool read_launchBtn() { return digitalRead(LAUNCH_BUTTON_PIN) == LOW; }
inline bool read_resetBtn() { return digitalRead(RESET_SYSTEM_PIN) == LOW; }

// ====== Celebration (non-blocking 3× blink of ALL lights) ======
void start_celebration(unsigned long nowMs)
{
    st.celebActive = true;
    st.celebStep = 0;
    st.celebLastToggle = nowMs;
    setAllLights(LOW);
    // LCD: show reset success (16x2 format)
    lcd.clear();
    lcd_center_text(0, "RESET COMPLETE");
    lcd_center_text(1, "SYSTEM SAFE");
    
    // Success sound
    buzzer_success();
}

void update_celebration(unsigned long nowMs)
{
    if (!st.celebActive) return;
    if (nowMs - st.celebLastToggle >= CELEB_BLINK_MS)
    {
        st.celebLastToggle = nowMs;
        const bool on = (st.celebStep % 2) == 0;
        setAllLights(on ? HIGH : LOW);
        st.celebStep++;
        if (st.celebStep >= CELEB_CYCLES * 2)
        {
            // Finalize reset to SAFE/idle
            st.celebActive = false;
            st.launchLatched = false;
            st.countdownActive = false;
            st.resetActive = false;
            st.resetLightsOverride = false;
            st.armBlinkOverride = false;
            reset_launch_indicators();
            st.armed = false; // come up SAFE
            set_armed_leds(st.armed); // NOT_ARMED on, ARMED off
            // LCD go to READY screen (2-line format)
            lcd.clear();
            lcd_center_text(0, "SYSTEM READY");
            lcd_center_text(1, "READY TO ARM");
            // Boot done; we are in RUN already, keep it
            bootPhase = BootPhase::RUN;
        }
    }
}

// ====== Reset FSM (post-launch only) ======
void update_reset_fsm(unsigned long nowMs)
{
    if (bootPhase != BootPhase::RUN || st.celebActive || st.activeLaunchLockout) return; // ignore during boot/celebration, active launch, or startup

    const bool resetHeld = read_resetBtn();

    // If we have NOT launched yet, ignore reset entirely.
    if (!st.launchLatched)
    {
        if (st.resetActive || st.resetLightsOverride)
        {
            st.resetActive = false;
            st.resetLightsOverride = false;
            set_armed_leds(st.armed);
        }
        if (st.armBlinkOverride)
        {
            st.armBlinkOverride = false;
            set_armed_leds(st.armed);
        }
        return;
    }

    // Post-launch only from here:

    // If ARMED, reset is forbidden: blink both indicators (long-on pattern)
    if (st.armed)
    {
        if (resetHeld)
        {
            if (!st.armBlinkOverride)
            {
                st.armBlinkOverride = true;
                st.armBlinkState = true; // start ON
                st.armBlinkLastToggle = nowMs;
                digitalWrite(ARMED_LED_PIN, HIGH);
                digitalWrite(NOT_ARMED_LED_PIN, HIGH);
                // LCD hint - clearer error message (16x2 format)
                lcd.clear();
                lcd_center_text(0, "RESET BLOCKED!");
                lcd_center_text(1, "DISARM FIRST");
                
                // Error sound
                buzzer_error();
            }
        }
        else if (st.armBlinkOverride)
        {
            st.armBlinkOverride = false;
            set_armed_leds(st.armed); // remains OFF post-launch
        }
        return; // cannot reset while armed
    }

    // DISARMED + post-launch: allow timed reset
    if (st.armBlinkOverride)
    {
        st.armBlinkOverride = false;
        set_armed_leds(st.armed);
    }

    if (resetHeld)
    {
        if (!st.resetActive)
        {
            st.resetActive = true;
            st.resetHoldStartMs = nowMs;
            st.resetLightsOverride = true;
            setAllLights(HIGH); // ALL lights on while holding reset (good case)
            
            // Stop reset reminder loop when reset is started
            st.resetReminderActive = false;
            
            // LCD: show resetting (16x2 format)
            lcd.clear();
            lcd_center_text(0, "HOLD RESET...");
            lcd_center_text(1, "DISARMED - OK");
        }
        else
        {
            if (!st.celebActive) setAllLights(HIGH);
            const unsigned long elapsed = nowMs - st.resetHoldStartMs;
            if (elapsed >= RESET_HOLD_MS)
            {
                st.resetActive = false;
                st.resetLightsOverride = false;
                start_celebration(nowMs);
            }
        }
    }
    else if (st.resetActive)
    {
        st.resetActive = false;
        st.resetLightsOverride = false;
        // restore post-launch visuals
        set_armed_leds(st.armed);
    }
}

// ====== Launch FSM ======
void update_launch_fsm(unsigned long nowMs)
{
    // Hard lockout: nothing launch-related once launched until reset completes
    // Also lockout during active launch phase or startup
    if (st.launchLatched || st.celebActive || st.resetLightsOverride || st.activeLaunchLockout || bootPhase != BootPhase::RUN) return;

    const bool buttonHeld = read_launchBtn();

    if (!st.armed)
    {
        reset_launch_indicators();
        return;
    }

    // Start countdown when button is first pressed
    if (buttonHeld && !st.countdownActive)
    {
        st.countdownActive = true;
        st.countdownStartMs = nowMs;
        setAll(COUNTDOWN_PINS, HIGH);
        digitalWrite(LAUNCH_IN_COUNTDOWN_PIN, HIGH);
        // LCD (16x2 format)
        lcd.clear();
        lcd_center_text(0, "COUNTDOWN ACTIVE");
        lcd_center_text(1, "T-5.0s HOLD");
        
        // Countdown start sound
        buzzer_tone(1000, 300); // High pitch start sound
    }
    
    // Continue countdown while button is held
    if (buttonHeld && st.countdownActive && !st.launchLatched)
    {
        const unsigned long elapsed = nowMs - st.countdownStartMs;

        setProgressReverse(COUNTDOWN_PINS, elapsed, COUNTDOWN_MS);

        long remain_ms = (long)COUNTDOWN_MS - (long)elapsed;
        if (remain_ms < 0) remain_ms = 0;
        double remain = remain_ms / 1000.0;

        // Only update LCD every 200ms to avoid flashing
        static unsigned long lastLcdUpdate = 0;
        if (nowMs - lastLcdUpdate >= 200)
        {
            lastLcdUpdate = nowMs;
            
            // Update countdown display on LCD
            lcd.clear();
            lcd_center_text(0, "COUNTDOWN ACTIVE");
            
            // Show countdown time on line 2
            lcd.setCursor(0, 1);
            lcd.print("T-");
            lcd.print(remain, 1); // Print with 1 decimal place
            lcd.print("s");
            
            // Add countdown tick sound every second
            if (remain >= 1.0 && remain <= 5.0) {
                static uint8_t lastSecond = 6;
                uint8_t currentSecond = (uint8_t)remain;
                if (currentSecond != lastSecond) {
                    buzzer_countdown_tick();
                    lastSecond = currentSecond;
                }
            }
            
            // Play warning siren during countdown (overrides normal siren)
            buzzer_warning_siren();
        }
        
        // Debug: Check if countdown should complete
        if (elapsed >= COUNTDOWN_MS - 100) // Within 100ms of completion
        {
            // Force display update near completion
            lcd.clear();
            lcd_center_text(0, "COUNTDOWN ACTIVE");
            lcd_center_text(1, "T-0.0s");
        }

        if (elapsed >= COUNTDOWN_MS)
        {
            // Countdown complete - launch sequence
            st.countdownActive = false;
            st.launchLatched = true;
            digitalWrite(LAUNCH_IN_COUNTDOWN_PIN, LOW);
            digitalWrite(LAUNCH_IN_LAUNCH_PIN, HIGH);
            set_armed_leds(st.armed);

            lcd.clear();
            lcd_center_text(0, "LAUNCH!");
            lcd_center_text(1, "IN ACTIVE LAUNCH");
            
            // Launch alarm
            buzzer_launch_alarm();
            
            // Start reset reminder loop
            start_reset_reminder_loop();
            
            // Activate igniter relay
            start_igniter_relay();
            
            // Force immediate display update
            lcd.clear();
            lcd_center_text(0, "LAUNCH!");
            lcd_center_text(1, "IN ACTIVE LAUNCH");
        }
    }
    // CRITICAL: Cancel countdown if button is released (but only before launch)
    else if (st.countdownActive && !st.launchLatched)
    {
        // Only cancel countdown if launch hasn't occurred
        st.countdownActive = false;
        digitalWrite(LAUNCH_IN_COUNTDOWN_PIN, LOW);
        setAll(COUNTDOWN_PINS, LOW);
        lcd.clear();
        lcd_center_text(0, "COUNTDOWN CANCEL");
        lcd_center_text(1, "SYSTEM READY");
    }

}

// ====== Boot/LCD FSM ======
void lcd_show_splash()
{
    lcd.clear();
    delay(100); // Ensure clear completes
    
    // Simple, reliable splash screen (16x2 format)
    lcd.setCursor(0, 0);
    lcd.print("Luke's Rocket");
    lcd.setCursor(0, 1);
    lcd.print("Launcher v0.1");
}

void lcd_show_ready()
{
    lcd.clear();
    lcd_center_text(0, "SYSTEM READY");
    lcd_center_text(1, "READY TO ARM");
    
    // Now that system is ready, activate the LED indicators
    st.armed = read_armed();
    set_armed_leds(st.armed);
    
    // Play ready sound
    buzzer_ready();
}

void update_boot_fsm(unsigned long now)
{
    switch (bootPhase)
    {
    case BootPhase::SPLASH:
        {
            if (bootPhaseStart == 0)
            {
                bootPhaseStart = now;
                lcd_show_splash();
            }
            if (now - bootPhaseStart >= SPLASH_MS)
            {
                bootPhase = BootPhase::SELFTEST;
                bootPhaseStart = now;
                lcd.clear();
            }
            break;
        }
            case BootPhase::SELFTEST:
        {
            // Show rolling self-test items over 6s
            unsigned long elapsed = now - bootPhaseStart;
            if (elapsed >= SELFTEST_MS)
            {
                bootPhase = BootPhase::READY_SHOWN;
                bootPhaseStart = now;
                lcd_show_ready();
                break;
            }
            
            // Only update display every 800ms to avoid flickering
            static unsigned long lastUpdate = 0;
            if (now - lastUpdate < 800) break;
            lastUpdate = now;
            
            // Which item are we on?
            size_t idx = (size_t)((elapsed * SELFTEST_COUNT) / SELFTEST_MS);
            if (idx >= SELFTEST_COUNT) idx = SELFTEST_COUNT - 1;

            // Simple, reliable display format (16x2)
            lcd.clear();
            lcd_write_line(0, "SYSTEM STARTUP", true);
            
            // Show current test item with progress
            String testItem = String(SELFTEST_ITEMS[idx]);
            int percent = (int)((elapsed * 100) / SELFTEST_MS);
            lcd_write_line(1, testItem + " " + String(percent) + "%", false);
            
            // Beep for each startup check
            buzzer_startup_check();
            break;
        }
    case BootPhase::READY_SHOWN:
        {
            // Hold the READY screen briefly (or go straight to RUN)
            // You asked to end at SYSTEM READY / READY TO ARM; keep showing it and proceed to RUN logic
            bootPhase = BootPhase::RUN;
            break;
        }
    case BootPhase::RUN: default:
        // nothing; normal FSMs take over
        break;
    }
}

// ====== Button Press Detection ======
void check_button_presses()
{
    // Don't check buttons during active launch lockout or startup
    if (st.activeLaunchLockout || bootPhase != BootPhase::RUN) return;
    
    static bool lastArmState = false;
    static bool lastLaunchState = false;
    static bool lastResetState = false;

    bool currentArmState = read_armed();
    bool currentLaunchState = read_launchBtn();
    bool currentResetState = read_resetBtn();

    // Arm switch pressed (transition from HIGH to LOW)
    if (currentArmState && !lastArmState)
    {
        lcd_show_button_feedback("ARMING SYSTEM");
        buzzer_arming();
    }
    // Arm switch released (transition from LOW to HIGH)
    else if (!currentArmState && lastArmState)
    {
        lcd_show_button_feedback("DISARMING SYSTEM");
        buzzer_disarming();
    }

    // Launch button pressed
    if (currentLaunchState && !lastLaunchState)
    {
        if (st.armed)
        {
            lcd_show_button_feedback("LAUNCH BUTTON");
        }
        else
        {
            lcd_show_button_feedback("SYSTEM NOT ARMED");
        }
    }

    // Reset button pressed
    if (currentResetState && !lastResetState)
    {
        if (st.launchLatched)
        {
            if (st.armed)
            {
                lcd_show_button_feedback("RESET BLOCKED");
            }
            else
            {
                lcd_show_button_feedback("RESET BUTTON");
            }
        }
        else
        {
            lcd_show_button_feedback("RESET BUTTON");
        }
    }

    lastArmState = currentArmState;
    lastLaunchState = currentLaunchState;
    lastResetState = currentResetState;
}

// ====== Arduino ======
void setup()
{
    init_pins();

    // LCD init with proper delays and reset sequence
    lcd.begin(LCD_COLS, LCD_ROWS);
    delay(LCD_INIT_DELAY);
    
    // Reset LCD to known state
    lcd.clear();
    delay(100);
    lcd.home();
    delay(50);
    
    // Simple LCD test - just basic text (16x2 format)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD TEST");
    lcd.setCursor(0, 1);
    lcd.print("LINE 1");
    delay(1500); // Show test for 1.5 seconds
    
    // Clear and prepare for splash screen
    lcd.clear();
    delay(100);
    bootPhaseStart = 0; // force splash render

    // Lights initial - keep all off during startup
    st.armed = false;
    // Don't set LEDs yet - wait until system is ready
    reset_launch_indicators();
}

void loop()
{
    const unsigned long now = millis();

    // Boot/LCD sequence runs first and can gate the rest until RUN
    update_boot_fsm(now);

    // Track arming; indicators respect lockout/overrides
    st.armed = read_armed();
    if (!st.resetLightsOverride && !st.celebActive && !st.activeLaunchLockout && bootPhase == BootPhase::RUN)
    {
        set_armed_leds(st.armed);
    }

    // Check for button presses and show feedback
    check_button_presses();

    // Clear button feedback if time has elapsed
    lcd_clear_button_feedback();

    // Normal FSMs (some are gated by boot/lockout)
    update_launch_fsm(now);
    update_reset_fsm(now);

    // Illegal reset warning blink (post-launch only)
    apply_arm_blink_override(now);

    // After successful launch (and not in celebration/override), pulse the progress LEDs
    if (st.launchLatched && !st.resetLightsOverride && !st.celebActive && bootPhase == BootPhase::RUN)
    {
        pulseAll(COUNTDOWN_PINS, now);
    }
    
    // Warning siren while armed (but not during countdown or launch)
    if (st.armed && !st.countdownActive && !st.launchLatched && bootPhase == BootPhase::RUN)
    {
        static unsigned long lastSirenUpdate = 0;
        if (now - lastSirenUpdate >= 500) // Update siren every 500ms
        {
            lastSirenUpdate = now;
            buzzer_warning_siren();
        }
    }

    // Celebration sequence (non-blocking)
    update_celebration(now);
    
    // Reset reminder loop (plays tune every 2 seconds for 10 seconds total)
    if (st.resetReminderActive)
    {
        const unsigned long elapsed = now - st.resetReminderStartMs;
        if (elapsed >= 10000) // Stop after 10 seconds
        {
            st.resetReminderActive = false;
        }
        else if (now - st.lastReminderPlay >= 2000) // Play every 2 seconds
        {
            st.lastReminderPlay = now;
            buzzer_reset_reminder();
        }
    }
    
    // Igniter relay control (activate for 10 seconds on launch)
    if (st.igniterRelayActive)
    {
        const unsigned long launchElapsed = now - st.igniterRelayStartMs;
        
        // CRITICAL: Keep relay HIGH for entire 10 seconds
        digitalWrite(IGNIGHTER_RELAY_PIN, HIGH);
        
        // Debug: Verify relay state and force it back on if needed
        if (digitalRead(IGNIGHTER_RELAY_PIN) != HIGH) {
            // Relay was turned off by something else - turn it back on immediately
            digitalWrite(IGNIGHTER_RELAY_PIN, HIGH);
        }
        
        // Additional safety: ensure relay stays HIGH
        static unsigned long lastRelayCheck = 0;
        if (now - lastRelayCheck >= 100) // Check every 100ms
        {
            lastRelayCheck = now;
            digitalWrite(IGNIGHTER_RELAY_PIN, HIGH);
        }
        
        // Update display and play sound during active launch
        static unsigned long lastActiveLaunchUpdate = 0;
        if (now - lastActiveLaunchUpdate >= 500) // Update every 500ms
        {
            lastActiveLaunchUpdate = now;
            
            // Update display to show countdown
            lcd.clear();
            lcd_center_text(0, "ACTIVE LAUNCH");
            lcd_center_text(1, "T+" + String(launchElapsed / 1000) + "s");
            
            // Play continuous active launch sound
            buzzer_active_launch();
        }
        
        // CRITICAL: Only transition to LAUNCH COMPLETE after FULL 10 seconds
        // Use >= 10000 to ensure we get the full 10 seconds
        if (launchElapsed >= 10000) // Exactly 10 seconds (10000ms)
        {
            // Final relay activation before deactivation
            digitalWrite(IGNIGHTER_RELAY_PIN, HIGH);
            
            st.igniterRelayActive = false;
            st.activeLaunchLockout = false; // Release system lockout
            deactivate_igniter_relay();
            
            // Update display to show launch complete
            lcd.clear();
            lcd_center_text(0, "LAUNCH COMPLETE");
            lcd_center_text(1, "SYSTEM LOCKED");
        }
    }

    delay(LOOP_SETTLE_MS);
}

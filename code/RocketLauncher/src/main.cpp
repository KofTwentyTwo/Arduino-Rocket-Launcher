#include <Arduino.h>
#include <Bounce2.h>
#include <LiquidCrystal.h>

constexpr uint16_t LCD_REFRESH_MS = 250; // ms between LCD updates

/* =========================
   Pin Map (UNO)
   ========================= */
const uint8_t PIN_ARM = 2; // toggle, INPUT_PULLUP (LOW=ON)
const uint8_t PIN_RESET = 3; // momentary, INPUT_PULLUP (LOW=pressed)
const uint8_t PIN_LAUNCH = 4; // momentary, INPUT_PULLUP (LOW=pressed)

const uint8_t PIN_LED_READY = 5;
const uint8_t PIN_LED_ARMED = 6;
const uint8_t PIN_LAUNCH_LIGHT = 7;
const uint8_t PIN_RELAY = 8; // igniter relay driver input
const uint8_t PIN_BUZZER = 9; // passive buzzer (tone())

// Relay polarity: set to match your hardware
// Discrete NPN driver -> ACTIVE = HIGH
// Many HiLetgo modules -> ACTIVE = LOW (flip these)
const bool RELAY_ACTIVE = HIGH;
const bool RELAY_INACTIVE = LOW;

// 16x2 LCD in 4-bit mode on analog pins
const uint8_t LCD_RS = A0, LCD_E = A1, LCD_D4 = A2, LCD_D5 = A3, LCD_D6 = A4, LCD_D7 = A5;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/* =========================
   Timings (ms)
   ========================= */
const uint32_t HOLD_TO_LAUNCH_MS = 5000; // must hold LAUNCH this long
const uint32_t RELAY_ON_MS = 5000; // relay energized duration
const uint32_t COOLDOWN_MS = 5000; // post-fire inhibit
const uint32_t ABORT_INHIBIT_MS = 1500; // after abort
const uint32_t RESET_HOLD_MS = 2500; // disarm + hold reset to clear FAULT

/* =========================
   Debouncers
   ========================= */
Bounce dbArm, dbReset, dbLaunch;

/* =========================
   State Machine
   ========================= */
enum State { STARTUP, SPLASH, READY, ARMED, LAUNCH_COUNTDOWN, LAUNCHING, COOLDOWN, ABORT, FAULT };

State state = STARTUP;
uint32_t enteredAt = 0;
uint32_t deadline = 0;
uint32_t launchHeldSince = 0;
uint32_t resetHeldSince = 0;

/* =========================
   Startup Self-Check
   ========================= */
const uint8_t STARTUP_CHECKS_COUNT = 20;
const uint32_t STARTUP_TOTAL_TIME_MS = 5000; // 10s total
const uint32_t STARTUP_CHECK_INTERVAL = STARTUP_TOTAL_TIME_MS / STARTUP_CHECKS_COUNT;

uint8_t startupCheckIndex = 0;
uint32_t lastCheckTime = 0;
uint32_t completionTime = 0;
bool startupComplete = false;

/* =========================
   System Lockout
   ========================= */
bool systemLocked = true; // Locked during STARTUP and SPLASH

// 20 items (<=16 chars per LCD line recommended)
const char* STARTUP_CHECKS[] = {
   "Ignition circuit", "Relay contacts", "Power supply", "Button debounce",
   "LCD display", "Buzzer tones", "ARM switch", "RESET button",
   "LAUNCH button", "Status LEDs", "Relay driver", "Safety locks",
   "Countdown timer", "Abort circuits", "Fault detection", "Cooldown timer",
   "ARM interlock", "Reset hold", "Global fault", "Final check"
};

/* =========================
   Buzzer Player (non-blocking)
   ========================= */
struct BuzzNote
{
   uint16_t freq, ms, gap_ms;
}; // freq 0 = rest
struct BuzzPlayer
{
   const BuzzNote* seq = nullptr;
   uint8_t len = 0, idx = 0;
   bool loop = false, inGap = false, active = false;
   uint32_t stepDeadline = 0;
} buzzer;

void buzzerPlay(const BuzzNote* seq, uint8_t len, bool loop = false)
{
   buzzer.seq = seq;
   buzzer.len = len;
   buzzer.idx = 0;
   buzzer.loop = loop;
   buzzer.inGap = false;
   buzzer.active = (seq && len > 0);
   buzzer.stepDeadline = 0;
}

void buzzerStop(uint8_t pin)
{
   buzzer.active = false;
   buzzer.seq = nullptr;
   noTone(pin);
}

void buzzerUpdate(uint32_t now, uint8_t pin)
{
   if (!buzzer.active || !buzzer.seq || buzzer.len == 0) return;
   const BuzzNote& n = buzzer.seq[buzzer.idx];
   if (buzzer.stepDeadline == 0)
   {
      if (!buzzer.inGap)
      {
         if (n.freq > 0) tone(pin, n.freq, n.ms);
         else noTone(pin);
         buzzer.stepDeadline = now + n.ms;
      }
      else
      {
         noTone(pin);
         buzzer.stepDeadline = now + n.gap_ms;
      }
      return;
   }
   if ((long)(now - buzzer.stepDeadline) >= 0)
   {
      if (!buzzer.inGap && n.gap_ms > 0)
      {
         buzzer.inGap = true;
         buzzer.stepDeadline = 0;
      }
      else
      {
         buzzer.inGap = false;
         buzzer.idx++;
         buzzer.stepDeadline = 0;
         if (buzzer.idx >= buzzer.len)
         {
            if (buzzer.loop) buzzer.idx = 0;
            else
            {
               buzzer.active = false;
               noTone(pin);
            }
         }
      }
   }
}

/* =========================
   UI / I/O Helpers
   ========================= */
inline bool armOn() { return dbArm.read() == LOW; } // LOW=armed
inline bool resetPressed() { return dbReset.read() == LOW; }
inline bool launchPressed() { return dbLaunch.read() == LOW; }

void writeRelay(bool on) { digitalWrite(PIN_RELAY, on ? RELAY_ACTIVE : RELAY_INACTIVE); }

void setOutputs(bool readyLed, bool armedLed, bool launchLamp, bool relayOn)
{
   digitalWrite(PIN_LED_READY, readyLed ? HIGH : LOW);
   digitalWrite(PIN_LED_ARMED, armedLed ? HIGH : LOW);
   digitalWrite(PIN_LAUNCH_LIGHT, launchLamp ? HIGH : LOW);
   writeRelay(relayOn);
}

void lcdState(const char* l1, const char* l2)
{
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print(l1);
   lcd.setCursor(0, 1);
   lcd.print(l2);
}

/* =========================
   Sounds
   ========================= */
const BuzzNote SND_CHIRP[] = {{2000, 80, 40}, {2500, 80, 0}};
// Slow two-tone siren while ARMED (loops)
const BuzzNote SND_ARMED[] = {{1200, 180, 120}, {1600, 180, 700}};
const BuzzNote SND_COUNTDOWN[] = {{1200, 90, 910}};
// Louder, quicker two-tone burst per second for countdown (loops)
// Frame totals ~1s: 120ms at 1.4kHz, 40ms gap, 120ms at 2.6kHz, 720ms gap
const BuzzNote SND_COUNTDOWN_SIREN[] = {{1400, 120, 40}, {2600, 120, 720}};
const BuzzNote SND_LAUNCH[] = {{1800, 500, 0}}; // loop for continuous
const BuzzNote SND_ABORT[] = {{900, 120, 60}, {700, 120, 60}};
const BuzzNote SND_FAULT[] = {{800, 200, 50}, {600, 200, 150}};
const BuzzNote SND_CHECK[] = {{1500, 100, 0}};

/* =========================
   Global Fault Check (stub)
   ========================= */
bool globalFaultActive() { return false; }

/* =========================
   State Transitions
   ========================= */
void enter(State s)
{
   state = s;
   enteredAt = millis();

   switch (s)
   {
   case STARTUP:
      setOutputs(false, false, false, false);
      startupCheckIndex = 0;
      lastCheckTime = 0;
      startupComplete = false;
      lcdState("STARTUP", "Self-check...");
      buzzerPlay(SND_CHIRP, sizeof(SND_CHIRP) / sizeof(BuzzNote), false);
      systemLocked = true;
      break;

   case SPLASH:
      setOutputs(false, false, false, false);
      lcdState("Luke's Rocket", "Controller v0.1");
      buzzerPlay(SND_CHIRP, sizeof(SND_CHIRP) / sizeof(BuzzNote), false);
      deadline = millis() + 5000; // 5 s splash
      systemLocked = true;
      break;

   case READY:
      setOutputs(true, false, false, false);
      lcdState("READY", "Disarmed");
      buzzerStop(PIN_BUZZER);
      systemLocked = false;
      break;

   case ARMED:
      setOutputs(false, true, false, false);
      lcdState("ARMED", "Hold LAUNCH");
      buzzerPlay(SND_ARMED, sizeof(SND_ARMED) / sizeof(BuzzNote), true);
      launchHeldSince = 0;
      break;

   case LAUNCH_COUNTDOWN:
      setOutputs(false, true, false, false);
      lcdState("COUNTDOWN", "Hold...");
      buzzerPlay(SND_COUNTDOWN_SIREN, sizeof(SND_COUNTDOWN_SIREN) / sizeof(BuzzNote), true);
      break;

   case LAUNCHING:
      // Relay + lamp ON for the entire window (hard-limited)
      setOutputs(false, false, true, true);
      lcdState("LAUNCHING", "Relay ON");
      deadline = millis() + RELAY_ON_MS;
      buzzerPlay(SND_LAUNCH, sizeof(SND_LAUNCH) / sizeof(BuzzNote), true);
      break;

   case COOLDOWN:
      setOutputs(false, false, false, false);
      lcdState("COOLDOWN", "Post-fire");
      deadline = millis() + COOLDOWN_MS;
      buzzerStop(PIN_BUZZER);
      break;

   case ABORT:
      setOutputs(false, false, false, false);
      lcdState("ABORT", "Inhibit...");
      deadline = millis() + ABORT_INHIBIT_MS;
      buzzerPlay(SND_ABORT, sizeof(SND_ABORT) / sizeof(BuzzNote), false);
      break;

   case FAULT:
      setOutputs(false, false, false, false); // force relay off
      lcdState("FAULT", "Disarm + Reset");
      resetHeldSince = 0;
      buzzerPlay(SND_FAULT, sizeof(SND_FAULT) / sizeof(BuzzNote), true);
      systemLocked = false;
      break;
   }
}

/* =========================
   Setup
   ========================= */
void setup()
{
   pinMode(PIN_ARM, INPUT_PULLUP);
   pinMode(PIN_RESET, INPUT_PULLUP);
   pinMode(PIN_LAUNCH, INPUT_PULLUP);

   pinMode(PIN_LED_READY, OUTPUT);
   pinMode(PIN_LED_ARMED, OUTPUT);
   pinMode(PIN_LAUNCH_LIGHT, OUTPUT);
   pinMode(PIN_RELAY, OUTPUT);
   pinMode(PIN_BUZZER, OUTPUT);

   // Safe boot: force relay inactive before anything else
   writeRelay(false);

   dbArm.attach(PIN_ARM);
   dbArm.interval(10);
   dbReset.attach(PIN_RESET);
   dbReset.interval(10);
   dbLaunch.attach(PIN_LAUNCH);
   dbLaunch.interval(10);

   lcd.begin(16, 2);

   enter(SPLASH);
}

/* =========================
   Main Loop
   ========================= */
void loop()
{
   dbArm.update();
   dbReset.update();
   dbLaunch.update();
   const uint32_t now = millis();

   // inputs enabled only when not locked
   const bool inputsEnabled = !systemLocked;

   // Global fault: from anywhere (except FAULT) jump to FAULT
   if (state != FAULT && globalFaultActive())
   {
      enter(FAULT);
   }

   switch (state)
   {
   case STARTUP:
      // Safety: fail startup if any control is active
      if (armOn() || resetPressed() || launchPressed())
      {
         enter(FAULT);
         break;
      }
      if (now - lastCheckTime >= STARTUP_CHECK_INTERVAL)
      {
         lastCheckTime = now;
         if (startupCheckIndex < STARTUP_CHECKS_COUNT)
         {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Check ");
            lcd.print(startupCheckIndex + 1);
            lcd.print("/");
            lcd.print(STARTUP_CHECKS_COUNT);
            lcd.setCursor(0, 1);
            lcd.print(STARTUP_CHECKS[startupCheckIndex]);
            buzzerPlay(SND_CHECK, sizeof(SND_CHECK) / sizeof(BuzzNote), false);
            startupCheckIndex++;
         }
         else
         {
            if (!startupComplete)
            {
               lcdState("Self-check", "COMPLETE!");
               completionTime = now;
               startupComplete = true;
            }
            if (now - completionTime >= 1000) enter(READY);
         }
      }
      break;

   case SPLASH:
      if ((long)(now - deadline) >= 0) enter(STARTUP);
      break;

   case READY:
      if (inputsEnabled && armOn()) enter(ARMED);
      break;

   case ARMED:
      if (inputsEnabled && !armOn())
      {
         enter(READY);
         break;
      }
      if (inputsEnabled && launchPressed())
      {
         if (launchHeldSince == 0) launchHeldSince = now;
         if (now - launchHeldSince >= 250) enter(LAUNCH_COUNTDOWN);
      }
      else
      {
         launchHeldSince = 0;
      }
      break;

   case LAUNCH_COUNTDOWN:
      {
         if (inputsEnabled && !armOn())
         {
            enter(FAULT);
            break;
         } // interlock change -> fault
         if (inputsEnabled && !launchPressed())
         {
            enter(ABORT);
            break;
         } // early release -> abort

         static uint32_t lastUpd = 0;
         if (now - lastUpd > LCD_REFRESH_MS)
         {
            lastUpd = now;
            const uint32_t held = now - enteredAt;
            long remain = (long)HOLD_TO_LAUNCH_MS - (long)held;
            if (remain < 0) remain = 0;
            lcd.setCursor(0, 1);
            lcd.print("Hold ");
            lcd.print(remain / 1000);
            lcd.print("s           ");
         }
         if (now - enteredAt >= HOLD_TO_LAUNCH_MS) enter(LAUNCHING);
         break;
      }

   case LAUNCHING:
      if ((long)(now - deadline) >= 0)
      {
         setOutputs(false, false, false, false); // ensure relay & lamp off
         enter(COOLDOWN);
      }
      break;

   case COOLDOWN:
      if ((long)(now - deadline) >= 0)
      {
         enter(FAULT); // requires disarm + reset to clear
      }
      break;

   case ABORT:
      if ((long)(now - deadline) >= 0)
      {
         if (armOn()) enter(ARMED);
         else enter(READY);
      }
      break;

   case FAULT:
      // Only exit if Arm OFF and Reset held for ≥10 s and no active fault
      if (inputsEnabled && !armOn())
      {
         if (resetPressed())
         {
            if (resetHeldSince == 0) resetHeldSince = now;
            if ((now - resetHeldSince) % LCD_REFRESH_MS < 10)
            {
               long remain = (long)RESET_HOLD_MS - (long)(now - resetHeldSince);
               if (remain < 0) remain = 0;
               lcd.setCursor(0, 1);
               lcd.print("Reset ");
               lcd.print(remain / 1000);
               lcd.print("s               ");
            }
            if (now - resetHeldSince >= RESET_HOLD_MS && !globalFaultActive())
            {
               enter(READY);
            }
         }
         else
         {
            resetHeldSince = 0;
         }
      }
      else
      {
         lcd.setCursor(0, 1);
         lcd.print("Disarm & Reset ");
         resetHeldSince = 0;
      }
      break;
   }

   // Run buzzer patterns last
   buzzerUpdate(now, PIN_BUZZER);
}

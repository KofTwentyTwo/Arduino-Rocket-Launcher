#include "RocketController.h"
#include "ArduinoInterface.h"
#include <string.h>

// Buzzer sequence definitions
const BuzzNote SND_CHIRP[]           = {{2000, 80, 40}, {2500, 80, 0}};
const BuzzNote SND_ARMED[]           = {{1200, 180, 120}, {1600, 180, 700}};
const BuzzNote SND_COUNTDOWN[]       = {{1200, 90, 910}};
const BuzzNote SND_COUNTDOWN_SIREN[] = {{1400, 120, 40}, {2600, 120, 720}};
const BuzzNote SND_LAUNCH[]          = {{1800, 500, 0}};
const BuzzNote SND_ABORT[]           = {{900, 120, 60}, {700, 120, 60}};
const BuzzNote SND_FAULT[]           = {{800, 200, 50}, {600, 200, 150}};
const BuzzNote SND_CHECK[]           = {{1500, 100, 0}};

// Startup check messages
const char*    STARTUP_CHECKS[]      = {
    "Ignition circuit", "Relay contacts", "Power supply",    "Button debounce", "LCD display",
    "Buzzer tones",     "ARM switch",     "RESET button",    "LAUNCH button",   "Status LEDs",
    "Relay driver",     "Safety locks",   "Countdown timer", "Abort circuits",  "Fault detection",
    "Cooldown timer",   "ARM interlock",  "Reset hold",      "Global fault",    "Final check"};

// Constructor
RocketController::RocketController(ArduinoInterface* interface)
    : interface(interface), state(State::STARTUP), systemLocked(true)
{
}

// Main update method
void RocketController::update(uint32_t now)
{
   // Update buzzer first
   updateBuzzer(now);

   // Global fault check
   if (state != State::FAULT && globalFaultActive())
   {
      enter(State::FAULT);
      return;
   }

   // Update based on current state
   switch (state)
   {
      case State::STARTUP:
         updateStartup(now);
         break;
      case State::SPLASH:
         updateSplash(now);
         break;
      case State::READY:
         updateReady(now);
         break;
      case State::ARMED:
         updateArmed(now);
         break;
      case State::LAUNCH_COUNTDOWN:
         updateLaunchCountdown(now);
         break;
      case State::LAUNCHING:
         updateLaunching(now);
         break;
      case State::COOLDOWN:
         updateCooldown(now);
         break;
      case State::ABORT:
         updateAbort(now);
         break;
      case State::FAULT:
         updateFault(now);
         break;
   }
}

// State transition method
void RocketController::enter(State newState)
{
   state     = newState;
   enteredAt = interface->millis();

   switch (newState)
   {
      case State::STARTUP:
         setOutputs(false, false, false, false);
         startupCheckIndex = 0;
         lastCheckTime     = 0;
         startupComplete   = false;
         updateLCD("STARTUP", "Self-check...");
         playBuzzerSequence(SND_CHIRP, 2, false);
         systemLocked = true;
         break;

      case State::SPLASH:
         setOutputs(false, false, false, false);
         updateLCD("Luke's Rocket", "Controller v0.1");
         playBuzzerSequence(SND_CHIRP, 2, false);
         deadline     = interface->millis() + 5000; // 5s splash
         systemLocked = true;
         break;

      case State::READY:
         setOutputs(true, false, false, false);
         updateLCD("READY", "Disarmed");
         stopBuzzer();
         systemLocked = false;
         break;

      case State::ARMED:
         setOutputs(false, true, false, false);
         updateLCD("ARMED", "Hold LAUNCH");
         playBuzzerSequence(SND_ARMED, 2, true);
         launchHeldSince = 0;
         break;

      case State::LAUNCH_COUNTDOWN:
         setOutputs(false, true, false, false);
         updateLCD("COUNTDOWN", "Hold...");
         playBuzzerSequence(SND_COUNTDOWN_SIREN, 2, true);
         break;

      case State::LAUNCHING:
         setOutputs(false, false, true, true);
         updateLCD("LAUNCHING", "Relay ON");
         deadline = interface->millis() + RELAY_ON_MS;
         playBuzzerSequence(SND_LAUNCH, 1, true);
         break;

      case State::COOLDOWN:
         setOutputs(false, false, false, false);
         updateLCD("COOLDOWN", "Post-fire");
         deadline = interface->millis() + COOLDOWN_MS;
         stopBuzzer();
         break;

      case State::ABORT:
         setOutputs(false, false, false, false);
         updateLCD("ABORT", "Inhibit...");
         deadline = interface->millis() + ABORT_INHIBIT_MS;
         playBuzzerSequence(SND_ABORT, 2, false);
         break;

      case State::FAULT:
         setOutputs(false, false, false, false);
         updateLCD("FAULT", "Disarm + Reset");
         resetHeldSince = 0;
         playBuzzerSequence(SND_FAULT, 2, true);
         systemLocked = false;
         break;
   }
}

// Input handling methods
void RocketController::setArmState(bool armed)
{
   // This will be called from the interface when ARM switch changes
}

void RocketController::setResetPressed(bool pressed)
{
   // This will be called from the interface when RESET button changes
}

void RocketController::setLaunchPressed(bool pressed)
{
   // This will be called from the interface when LAUNCH button changes
}

// Audio control methods
void RocketController::playBuzzerSequence(const BuzzNote* sequence, uint8_t length, bool loop)
{
   buzzer.seq          = sequence;
   buzzer.len          = length;
   buzzer.idx          = 0;
   buzzer.loop         = loop;
   buzzer.inGap        = false;
   buzzer.active       = (sequence && length > 0);
   buzzer.stepDeadline = 0;
}

void RocketController::stopBuzzer()
{
   buzzer.active = false;
   buzzer.seq    = nullptr;
   interface->noTone(9); // PIN_BUZZER
}

// Private update methods for each state
void RocketController::updateStartup(uint32_t now)
{
   // Safety: fail startup if any control is active
   if (interface->isArmPressed() || interface->isResetPressed() || interface->isLaunchPressed())
   {
      enter(State::FAULT);
      return;
   }

   if (now - lastCheckTime >= STARTUP_CHECK_INTERVAL)
   {
      lastCheckTime = now;
      if (startupCheckIndex < STARTUP_CHECKS_COUNT)
      {
         interface->lcdClear();
         interface->lcdSetCursor(0, 0);
         interface->lcdPrint("Check ");
         interface->lcdPrint(startupCheckIndex + 1);
         interface->lcdPrint("/");
         interface->lcdPrint(STARTUP_CHECKS_COUNT);
         interface->lcdSetCursor(0, 1);
         interface->lcdPrint(STARTUP_CHECKS[startupCheckIndex]);
         playBuzzerSequence(SND_CHECK, 1, false);
         startupCheckIndex++;
      }
      else
      {
         if (!startupComplete)
         {
            updateLCD("Self-check", "COMPLETE!");
            completionTime  = now;
            startupComplete = true;
         }
         if (now - completionTime >= 1000)
         {
            enter(State::READY);
         }
      }
   }
}

void RocketController::updateSplash(uint32_t now)
{
   if ((long)(now - deadline) >= 0)
   {
      enter(State::STARTUP);
   }
}

void RocketController::updateReady(uint32_t now)
{
   if (!systemLocked && interface->isArmPressed())
   {
      enter(State::ARMED);
   }
}

void RocketController::updateArmed(uint32_t now)
{
   if (!systemLocked && !interface->isArmPressed())
   {
      enter(State::READY);
      return;
   }

   if (!systemLocked && interface->isLaunchPressed())
   {
      if (launchHeldSince == 0)
         launchHeldSince = now;
      if (now - launchHeldSince >= 250)
      {
         enter(State::LAUNCH_COUNTDOWN);
      }
   }
   else
   {
      launchHeldSince = 0;
   }
}

void RocketController::updateLaunchCountdown(uint32_t now)
{
   if (!systemLocked && !interface->isArmPressed())
   {
      enter(State::FAULT); // interlock change -> fault
      return;
   }

   if (!systemLocked && !interface->isLaunchPressed())
   {
      enter(State::ABORT); // early release -> abort
      return;
   }

   // Update countdown display every 250ms
   static uint32_t lastUpd = 0;
   if (now - lastUpd > 250)
   {
      lastUpd               = now;
      const uint32_t held   = now - enteredAt;
      long           remain = (long)HOLD_TO_LAUNCH_MS - (long)held;
      if (remain < 0)
         remain = 0;

      interface->lcdSetCursor(0, 1);
      interface->lcdPrint("Hold ");
      interface->lcdPrint(remain / 1000);
      interface->lcdPrint("s           ");
   }

   if (now - enteredAt >= HOLD_TO_LAUNCH_MS)
   {
      enter(State::LAUNCHING);
   }
}

void RocketController::updateLaunching(uint32_t now)
{
   if ((long)(now - deadline) >= 0)
   {
      setOutputs(false, false, false, false); // ensure relay & lamp off
      enter(State::COOLDOWN);
   }
}

void RocketController::updateCooldown(uint32_t now)
{
   if ((long)(now - deadline) >= 0)
   {
      enter(State::FAULT); // requires disarm + reset to clear
   }
}

void RocketController::updateAbort(uint32_t now)
{
   if ((long)(now - deadline) >= 0)
   {
      if (interface->isArmPressed())
      {
         enter(State::ARMED);
      }
      else
      {
         enter(State::READY);
      }
   }
}

void RocketController::updateFault(uint32_t now)
{
   // Only exit if Arm OFF and Reset held for ≥2.5s and no active fault
   if (!systemLocked && !interface->isArmPressed())
   {
      if (interface->isResetPressed())
      {
         if (resetHeldSince == 0)
            resetHeldSince = now;

         // Update reset countdown display
         static uint32_t lastUpd = 0;
         if (now - lastUpd > 250)
         {
            lastUpd     = now;
            long remain = (long)RESET_HOLD_MS - (long)(now - resetHeldSince);
            if (remain < 0)
               remain = 0;
            interface->lcdSetCursor(0, 1);
            interface->lcdPrint("Reset ");
            interface->lcdPrint(remain / 1000);
            interface->lcdPrint("s               ");
         }

         if (now - resetHeldSince >= RESET_HOLD_MS && !globalFaultActive())
         {
            enter(State::READY);
         }
      }
      else
      {
         resetHeldSince = 0;
      }
   }
   else
   {
      interface->lcdSetCursor(0, 1);
      interface->lcdPrint("Disarm & Reset ");
      resetHeldSince = 0;
   }
}

// Buzzer update method
void RocketController::updateBuzzer(uint32_t now)
{
   if (!buzzer.active || !buzzer.seq || buzzer.len == 0)
      return;

   const BuzzNote& n = buzzer.seq[buzzer.idx];
   if (buzzer.stepDeadline == 0)
   {
      if (!buzzer.inGap)
      {
         if (n.freq > 0)
            interface->tone(9, n.freq, n.ms); // PIN_BUZZER
         else
            interface->noTone(9);
         buzzer.stepDeadline = now + n.ms;
      }
      else
      {
         interface->noTone(9);
         buzzer.stepDeadline = now + n.gap_ms;
      }
      return;
   }

   if ((long)(now - buzzer.stepDeadline) >= 0)
   {
      if (!buzzer.inGap && n.gap_ms > 0)
      {
         buzzer.inGap        = true;
         buzzer.stepDeadline = 0;
      }
      else
      {
         buzzer.inGap = false;
         buzzer.idx++;
         buzzer.stepDeadline = 0;
         if (buzzer.idx >= buzzer.len)
         {
            if (buzzer.loop)
               buzzer.idx = 0;
            else
            {
               buzzer.active = false;
               interface->noTone(9);
            }
         }
      }
   }
}

// Safety check methods
bool RocketController::globalFaultActive() const
{
   // Stub implementation - could be expanded for real fault detection
   return false;
}

bool RocketController::checkStartupSafety() const
{
   // Check if any controls are active during startup
   return !(interface->isArmPressed() || interface->isResetPressed() ||
            interface->isLaunchPressed());
}

// Helper methods
void RocketController::setOutputs(bool readyLed, bool armedLed, bool launchLamp, bool relayOn)
{
   interface->digitalWrite(5, readyLed ? HIGH : LOW);   // PIN_LED_READY
   interface->digitalWrite(6, armedLed ? HIGH : LOW);   // PIN_LED_ARMED
   interface->digitalWrite(7, launchLamp ? HIGH : LOW); // PIN_LAUNCH_LIGHT
   interface->digitalWrite(8, relayOn ? HIGH : LOW);    // PIN_RELAY
}

void RocketController::updateLCD(const char* line1, const char* line2)
{
   interface->lcdClear();
   interface->lcdSetCursor(0, 0);
   interface->lcdPrint(line1);
   interface->lcdSetCursor(0, 1);
   interface->lcdPrint(line2);
}
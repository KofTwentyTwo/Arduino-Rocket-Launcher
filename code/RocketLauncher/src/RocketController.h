#ifndef ROCKET_CONTROLLER_H
#define ROCKET_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations for hardware interface
class ArduinoInterface;

// State machine states
enum class State
{
   STARTUP,
   SPLASH,
   READY,
   ARMED,
   LAUNCH_COUNTDOWN,
   LAUNCHING,
   COOLDOWN,
   ABORT,
   FAULT
};

// Buzzer note structure
struct BuzzNote
{
   uint16_t freq;
   uint16_t ms;
   uint16_t gap_ms;
};

// Buzzer player for non-blocking audio
struct BuzzPlayer
{
   const BuzzNote* seq          = nullptr;
   uint8_t         len          = 0;
   uint8_t         idx          = 0;
   bool            loop         = false;
   bool            inGap        = false;
   bool            active       = false;
   uint32_t        stepDeadline = 0;
};

// Main rocket controller class
class RocketController
{
 public:
   // Constructor
   RocketController(ArduinoInterface* interface);

   // Main control methods
   void  update(uint32_t now);
   void  enter(State newState);

   // State queries
   State getState() const
   {
      return state;
   }

   bool isSystemLocked() const
   {
      return systemLocked;
   }

   bool isArmed() const
   {
      return state == State::ARMED;
   }

   bool isLaunching() const
   {
      return state == State::LAUNCHING;
   }

   // Input handling
   void setArmState(bool armed);
   void setResetPressed(bool pressed);
   void setLaunchPressed(bool pressed);

   // Audio control
   void playBuzzerSequence(const BuzzNote* sequence, uint8_t length, bool loop = false);
   void stopBuzzer();

   // Timing constants
   static constexpr uint32_t HOLD_TO_LAUNCH_MS      = 5000;
   static constexpr uint32_t RELAY_ON_MS            = 5000;
   static constexpr uint32_t COOLDOWN_MS            = 5000;
   static constexpr uint32_t ABORT_INHIBIT_MS       = 1500;
   static constexpr uint32_t RESET_HOLD_MS          = 2500;
   static constexpr uint8_t  STARTUP_CHECKS_COUNT   = 20;
   static constexpr uint32_t STARTUP_TOTAL_TIME_MS  = 5000;
   static constexpr uint32_t STARTUP_CHECK_INTERVAL = STARTUP_TOTAL_TIME_MS / STARTUP_CHECKS_COUNT;

 private:
   // Hardware interface
   ArduinoInterface* interface;

   // State machine
   State             state             = State::STARTUP;
   uint32_t          enteredAt         = 0;
   uint32_t          deadline          = 0;
   uint32_t          launchHeldSince   = 0;
   uint32_t          resetHeldSince    = 0;
   uint8_t           startupCheckIndex = 0;
   uint32_t          lastCheckTime     = 0;
   uint32_t          completionTime    = 0;
   bool              startupComplete   = false;

   // System state
   bool              systemLocked      = true;

   // Buzzer control
   BuzzPlayer        buzzer;

   // Internal methods
   void              updateBuzzer(uint32_t now);
   void              updateStartup(uint32_t now);
   void              updateSplash(uint32_t now);
   void              updateReady(uint32_t now);
   void              updateArmed(uint32_t now);
   void              updateLaunchCountdown(uint32_t now);
   void              updateLaunching(uint32_t now);
   void              updateCooldown(uint32_t now);
   void              updateAbort(uint32_t now);
   void              updateFault(uint32_t now);

   // Safety checks
   bool              globalFaultActive() const;
   bool              checkStartupSafety() const;

   // State transition helpers
   void              transitionTo(State newState);
   void              setOutputs(bool readyLed, bool armedLed, bool launchLamp, bool relayOn);
   void              updateLCD(const char* line1, const char* line2);
};

// Predefined buzzer sequences
extern const BuzzNote SND_CHIRP[];
extern const BuzzNote SND_ARMED[];
extern const BuzzNote SND_COUNTDOWN[];
extern const BuzzNote SND_COUNTDOWN_SIREN[];
extern const BuzzNote SND_LAUNCH[];
extern const BuzzNote SND_ABORT[];
extern const BuzzNote SND_FAULT[];
extern const BuzzNote SND_CHECK[];

#endif // ROCKET_CONTROLLER_H
#include <Arduino.h>
#include <Bounce2.h>
#include <LiquidCrystal.h>
#include "RocketController.h"
#include "ArduinoInterface.h"

// Real Arduino interface implementation
class RealArduinoInterface : public ArduinoInterface
{
 private:
   // Pin definitions
   static constexpr uint8_t PIN_ARM          = 2;
   static constexpr uint8_t PIN_RESET        = 3;
   static constexpr uint8_t PIN_LAUNCH       = 4;
   static constexpr uint8_t PIN_LED_READY    = 5;
   static constexpr uint8_t PIN_LED_ARMED    = 6;
   static constexpr uint8_t PIN_LAUNCH_LIGHT = 7;
   static constexpr uint8_t PIN_RELAY        = 8;
   static constexpr uint8_t PIN_BUZZER       = 9;

   // LCD pins (analog pins used as digital)
   static constexpr uint8_t LCD_RS           = A0;
   static constexpr uint8_t LCD_E            = A1;
   static constexpr uint8_t LCD_D4           = A2;
   static constexpr uint8_t LCD_D5           = A3;
   static constexpr uint8_t LCD_D6           = A4;
   static constexpr uint8_t LCD_D7           = A5;

   // Relay polarity
   static constexpr bool    RELAY_ACTIVE     = HIGH;
   static constexpr bool    RELAY_INACTIVE   = LOW;

   // Hardware objects
   LiquidCrystal*           lcd;
   Bounce*                  dbArm;
   Bounce*                  dbReset;
   Bounce*                  dbLaunch;

 public:
   RealArduinoInterface()
   {
      // Initialize hardware objects
      lcd      = new LiquidCrystal(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
      dbArm    = new Bounce();
      dbReset  = new Bounce();
      dbLaunch = new Bounce();

      // Setup pin modes
      pinMode(PIN_ARM, INPUT_PULLUP);
      pinMode(PIN_RESET, INPUT_PULLUP);
      pinMode(PIN_LAUNCH, INPUT_PULLUP);
      pinMode(PIN_LED_READY, OUTPUT);
      pinMode(PIN_LED_ARMED, OUTPUT);
      pinMode(PIN_LAUNCH_LIGHT, OUTPUT);
      pinMode(PIN_RELAY, OUTPUT);
      pinMode(PIN_BUZZER, OUTPUT);

      // Safe boot: force relay inactive
      digitalWrite(PIN_RELAY, RELAY_INACTIVE);

      // Setup debouncers
      dbArm->attach(PIN_ARM);
      dbArm->interval(10);
      dbReset->attach(PIN_RESET);
      dbReset->interval(10);
      dbLaunch->attach(PIN_LAUNCH);
      dbLaunch->interval(10);

      // Initialize LCD
      lcd->begin(16, 2);
   }

   ~RealArduinoInterface()
   {
      // Suppress warning about non-virtual destructors
      // We're deleting concrete objects, not through base pointers
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
      delete lcd;
      delete dbArm;
      delete dbReset;
      delete dbLaunch;
      #pragma GCC diagnostic pop
   }

   // Pin control
   void digitalWrite(uint8_t pin, uint8_t state) override
   {
      ::digitalWrite(pin, state);
   }

   uint8_t digitalRead(uint8_t pin) const override
   {
      return ::digitalRead(pin);
   }

   void pinMode(uint8_t pin, uint8_t mode) override
   {
      ::pinMode(pin, mode);
   }

   // Time functions
   uint32_t millis() const override
   {
      return ::millis();
   }

   void delay(uint32_t ms) override
   {
      ::delay(ms);
   }

   // Audio functions
   void tone(uint8_t pin, uint16_t freq) override
   {
      ::tone(pin, freq);
   }

   void tone(uint8_t pin, uint16_t freq, uint32_t duration) override
   {
      ::tone(pin, freq, duration);
   }

   void noTone(uint8_t pin) override
   {
      ::noTone(pin);
   }

   // LCD functions
   void lcdClear() override
   {
      lcd->clear();
   }

   void lcdSetCursor(uint8_t col, uint8_t row) override
   {
      lcd->setCursor(col, row);
   }

   void lcdPrint(const char* text) override
   {
      lcd->print(text);
   }

   void lcdPrint(int number) override
   {
      lcd->print(number);
   }

   // Button debouncing
   void updateDebouncers() override
   {
      dbArm->update();
      dbReset->update();
      dbLaunch->update();
   }

   bool isArmPressed() const override
   {
      return dbArm->read() == LOW;
   }

   bool isResetPressed() const override
   {
      return dbReset->read() == LOW;
   }

   bool isLaunchPressed() const override
   {
      return dbLaunch->read() == LOW;
   }
};

// Global objects
RealArduinoInterface* arduinoInterface;
RocketController*     rocketController;

void                  setup()
{
   // Create hardware interface
   arduinoInterface = new RealArduinoInterface();

   // Create rocket controller
   rocketController = new RocketController(arduinoInterface);

   // Start in SPLASH state
   rocketController->enter(State::SPLASH);
}

void loop()
{
   // Update hardware interface
   arduinoInterface->updateDebouncers();

   // Update rocket controller
   rocketController->update(arduinoInterface->millis());
}
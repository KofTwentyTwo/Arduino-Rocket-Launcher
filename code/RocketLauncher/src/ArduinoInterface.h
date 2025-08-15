#ifndef ARDUINO_INTERFACE_H
#define ARDUINO_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
// Test environment - provide minimal Arduino definitions
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Arduino types
typedef uint8_t byte;
typedef bool boolean;

// Arduino classes (forward declarations)
class LiquidCrystal;
class Bounce;
#endif

// Hardware abstraction interface
class ArduinoInterface
{
 public:
   virtual ~ArduinoInterface()                                          = default;

   // Pin control
   virtual void     digitalWrite(uint8_t pin, uint8_t state)            = 0;
   virtual uint8_t  digitalRead(uint8_t pin) const                      = 0;
   virtual void     pinMode(uint8_t pin, uint8_t mode)                  = 0;

   // Time functions
   virtual uint32_t millis() const                                      = 0;
   virtual void     delay(uint32_t ms)                                  = 0;

   // Audio functions
   virtual void     tone(uint8_t pin, uint16_t freq)                    = 0;
   virtual void     tone(uint8_t pin, uint16_t freq, uint32_t duration) = 0;
   virtual void     noTone(uint8_t pin)                                 = 0;

   // LCD functions
   virtual void     lcdClear()                                          = 0;
   virtual void     lcdSetCursor(uint8_t col, uint8_t row)              = 0;
   virtual void     lcdPrint(const char* text)                          = 0;
   virtual void     lcdPrint(int number)                                = 0;

   // Button debouncing (could be moved to controller if needed)
   virtual void     updateDebouncers()                                  = 0;
   virtual bool     isArmPressed() const                                = 0;
   virtual bool     isResetPressed() const                              = 0;
   virtual bool     isLaunchPressed() const                             = 0;
};

// Note: RealArduinoInterface is implemented in main.cpp

#endif // ARDUINO_INTERFACE_H

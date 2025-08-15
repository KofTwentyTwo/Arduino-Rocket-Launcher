#include <unity.h>
#include <cstring>
#include <cstdio>
#include "../src/RocketController.h"
#include "../src/ArduinoInterface.h"

// Simple mock Arduino interface for testing
class MockArduinoInterface : public ArduinoInterface
{
 private:
   // Mock state
   mutable uint32_t mock_millis = 0;
   uint8_t          mock_pin_states[20] = {0};
   bool             mock_tone_active = false;
   uint16_t         mock_tone_freq = 0;
   char             mock_lcd_line1[32] = "";
   char             mock_lcd_line2[32] = "";
   
   // Mock button states
   bool             mock_arm_pressed = false;
   bool             mock_reset_pressed = false;
   bool             mock_launch_pressed = false;

 public:
   // Pin control
   void digitalWrite(uint8_t pin, uint8_t state) override
   {
      mock_pin_states[pin] = state;
   }
   
   uint8_t digitalRead(uint8_t pin) const override
   {
      return mock_pin_states[pin];
   }
   
   void pinMode(uint8_t pin, uint8_t mode) override
   {
      (void)pin;   // Suppress unused parameter warning
      (void)mode;  // Suppress unused parameter warning
      // Mock implementation
   }
   
   // Time functions
   uint32_t millis() const override
   {
      return mock_millis;
   }
   
   void delay(uint32_t ms) override
   {
      mock_millis += ms;
   }
   
   // Audio functions
   void tone(uint8_t pin, uint16_t freq) override
   {
      (void)pin;   // Suppress unused parameter warning
      mock_tone_active = true;
      mock_tone_freq = freq;
   }
   
   void tone(uint8_t pin, uint16_t freq, uint32_t duration) override
   {
      (void)pin;      // Suppress unused parameter warning
      (void)duration; // Suppress unused parameter warning
      mock_tone_active = true;
      mock_tone_freq = freq;
   }
   
   void noTone(uint8_t pin) override
   {
      (void)pin;   // Suppress unused parameter warning
      mock_tone_active = false;
      mock_tone_freq = 0;
   }
   
   // LCD functions
   void lcdClear() override
   {
      mock_lcd_line1[0] = '\0';
      mock_lcd_line2[0] = '\0';
   }
   
   void lcdSetCursor(uint8_t col, uint8_t row) override
   {
      (void)col;  // Suppress unused parameter warning
      (void)row;  // Suppress unused parameter warning
      // Mock implementation
   }
   
   void lcdPrint(const char* text) override
   {
      if (strlen(mock_lcd_line1) == 0)
      {
         strncpy(mock_lcd_line1, text, 31);
      }
      else
      {
         strncpy(mock_lcd_line2, text, 31);
      }
   }
   
   void lcdPrint(int number) override
   {
      if (strlen(mock_lcd_line1) == 0)
      {
         snprintf(mock_lcd_line1, 32, "%d", number);
      }
      else
      {
         snprintf(mock_lcd_line2, 32, "%d", number);
      }
   }
   
   // Button debouncing
   void updateDebouncers() override
   {
      // Mock implementation
   }
   
   bool isArmPressed() const override
   {
      return mock_arm_pressed;
   }
   
   bool isResetPressed() const override
   {
      return mock_reset_pressed;
   }
   
   bool isLaunchPressed() const override
   {
      return mock_launch_pressed;
   }
   
   // Test helper methods
   void setMockTime(uint32_t time) { mock_millis = time; }
   void advanceTime(uint32_t ms) { mock_millis += ms; }
   void setArmPressed(bool pressed) { mock_arm_pressed = pressed; }
   void setResetPressed(bool pressed) { mock_reset_pressed = pressed; }
   void setLaunchPressed(bool pressed) { mock_launch_pressed = pressed; }
   
   // State query methods
   uint8_t getPinState(uint8_t pin) const { return mock_pin_states[pin]; }
   bool isToneActive() const { return mock_tone_active; }
   uint16_t getToneFreq() const { return mock_tone_freq; }
   const char* getLCDLine1() const { return mock_lcd_line1; }
   const char* getLCDLine2() const { return mock_lcd_line2; }
   
   // Reset mock state
   void reset()
   {
      mock_millis = 0;
      mock_tone_active = false;
      mock_tone_freq = 0;
      mock_lcd_line1[0] = '\0';
      mock_lcd_line2[0] = '\0';
      mock_arm_pressed = false;
      mock_reset_pressed = false;
      mock_launch_pressed = false;
      
      for (int i = 0; i < 20; i++)
      {
         mock_pin_states[i] = 0;
      }
   }
};

// Test fixture
MockArduinoInterface* mockInterface;
RocketController*     controller;

void setUp(void)
{
   mockInterface = new MockArduinoInterface();
   controller = new RocketController(mockInterface);
   mockInterface->reset();
}

void tearDown(void)
{
   delete controller;
   delete mockInterface;
}

// Test 1: Basic Rocket Controller Creation and Initial State
void test_rocket_controller_initial_state(void)
{
   // Test that controller starts in STARTUP state
   TEST_ASSERT_EQUAL(State::STARTUP, controller->getState());
   
   // Test that system is initially locked
   TEST_ASSERT_TRUE(controller->isSystemLocked());
   
   // Test that controller is not armed initially
   TEST_ASSERT_FALSE(controller->isArmed());
   
   // Test that controller is not launching initially
   TEST_ASSERT_FALSE(controller->isLaunching());
}

// Test 2: State Transition from STARTUP to READY
void test_startup_to_ready_transition(void)
{
   // Verify we start in STARTUP
   TEST_ASSERT_EQUAL(State::STARTUP, controller->getState());
   
   // The startup process takes:
   // - 20 startup checks × 250ms = 5000ms (5 seconds)
   // - Additional wait time = 2000ms (2 seconds)
   // - Total: 7000ms (7 seconds)
   
   uint32_t currentTime = 0;
   const uint32_t stepSize = 100; // Check every 100ms
   const uint32_t maxTime = 10000; // Max 10 seconds to be safe
   
   // Advance time until startup completes
   while (currentTime < maxTime && controller->getState() == State::STARTUP)
   {
      currentTime += stepSize;
      mockInterface->setMockTime(currentTime);
      controller->update(mockInterface->millis());
   }
   
   // Should now transition to READY state
   TEST_ASSERT_EQUAL(State::READY, controller->getState());
   
   // System should no longer be locked
   TEST_ASSERT_FALSE(controller->isSystemLocked());
   
   // Verify startup completed within reasonable time (should be around 7 seconds)
   TEST_ASSERT_LESS_OR_EQUAL(10000, currentTime);
}

// Test 3: Button Input Handling
void test_button_input_handling(void)
{
   // Test ARM button state
   mockInterface->setArmPressed(true);
   TEST_ASSERT_TRUE(mockInterface->isArmPressed());
   
   mockInterface->setArmPressed(false);
   TEST_ASSERT_FALSE(mockInterface->isArmPressed());
   
   // Test RESET button state
   mockInterface->setResetPressed(true);
   TEST_ASSERT_TRUE(mockInterface->isResetPressed());
   
   // Test LAUNCH button state
   mockInterface->setLaunchPressed(true);
   TEST_ASSERT_TRUE(mockInterface->isLaunchPressed());
}

// Test 4: Manual State Management
void test_manual_state_management(void)
{
   // Test that we can manually set states
   controller->enter(State::READY);
   TEST_ASSERT_EQUAL(State::READY, controller->getState());
   
   // Test that system is unlocked in READY state
   TEST_ASSERT_FALSE(controller->isSystemLocked());
   
   // Test ARMED state
   controller->enter(State::ARMED);
   TEST_ASSERT_EQUAL(State::ARMED, controller->getState());
   TEST_ASSERT_TRUE(controller->isArmed());
   TEST_ASSERT_FALSE(controller->isLaunching());
   
   // Test LAUNCHING state
   controller->enter(State::LAUNCHING);
   TEST_ASSERT_EQUAL(State::LAUNCHING, controller->getState());
   TEST_ASSERT_TRUE(controller->isLaunching());
   TEST_ASSERT_FALSE(controller->isArmed());
   
   // Test return to READY
   controller->enter(State::READY);
   TEST_ASSERT_EQUAL(State::READY, controller->getState());
   TEST_ASSERT_FALSE(controller->isArmed());
   TEST_ASSERT_FALSE(controller->isLaunching());
}

// Main test runner
void RUN_UNITY_TESTS()
{
   UNITY_BEGIN();
   
   // Rocket Controller Tests
   RUN_TEST(test_rocket_controller_initial_state);
   RUN_TEST(test_startup_to_ready_transition);
   RUN_TEST(test_button_input_handling);
   RUN_TEST(test_manual_state_management);
   
   UNITY_END();
}

// Main function for native testing
int main()
{
   // Initialize Unity test framework
   RUN_UNITY_TESTS();
   return 0;
}

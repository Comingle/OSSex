/* OSSex.h v0.3.1 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle
 * {♥} COMINGLE
*/


#ifndef OSSex_h
#define OSSex_h

#include <Arduino.h>
#include <avr/interrupt.h> 
#include <avr/io.h>
#include "OneButton.h"

// Timer4 is for ATmega32U4/Lilypad usb and should interrupt every ~1ms. Timer2 is for ATmega328P/Arduino Uno and should interrupt ~1ms.
#define TIMER4_INIT 16;
#define TIMER2_INIT 131;

// model names
 #define ALPHA 0
 #define BETA 1

class OSSex {
  public:
    OSSex();
    void setID(int);
    int setOutput(int, int);
    int setLED(int, int);
    int runShortPattern(int*, size_t);
    int runPattern(unsigned int);
    int runPattern(int* (*callback)(int));
    int cyclePattern();
    int addPattern(int* (*callback)(int));
    int getInput(int);
    int getPattern();
    void update();
    void attachClick(void (*callback)());
    void attachDoubleClick(void (*callback)());
    void attachLongPressStart(void (*callback)());
    void attachLongPressStop(void (*callback)());
    void attachDuringLongPress(void (*callback)());
    void setScale(float);
    void setPowerScale(float);
    float increasePower();
    float decreasePower();
    void setTimeScale(float);
    float increaseTime();
    float decreaseTime();
    void stop();
    int setHackerPort(unsigned int);

    static const int _max_outputs = 8;
    static const int _max_leds = 8;
    static const int _max_inputs = 4;
    struct {
      int deviceId;                     // device identifier number
      bool bothWays;                    // can outputs go both forward and backward?
      uint8_t outCount;                 // number of outputs (electrodes, motors)
      uint8_t outPins[_max_outputs];    // array mapping to output pins
      uint8_t tuoPins[_max_outputs];    // array mapping to reverse output pins;
      bool isLedMultiColor;             // do we have multicolored LEDs?
      uint8_t ledCount;                 // number of LEDs
      uint8_t ledPins[_max_leds];       // array mapping to LED output pins
      uint8_t inCount;                  // number of input pins
      uint8_t inPins[_max_inputs];      // array mapping to input pins
      struct {
        OneButton button;               // button object
        unsigned int pin;               // onboard pin
        unsigned int memAddress;        // EEPROM address for storing button state
      } buttons[1];
      uint8_t muxPins[2];               // pins for controlling hacker port multiplexer
    } device;
   
  private:
    struct pattern {
      int outNumber;
      uint8_t powerLevel;
      unsigned int duration;
      pattern *nextStep; 
    };
    pattern *_singlePattern;
    volatile pattern *_currentStep;

    struct patternList {
      int* (*patternFunc)(int);
      patternList *nextPattern;
    };
    volatile patternList *_currentPattern;
    volatile patternList *_first;
    int* (*_patternCallback)(int);

    float _powerScale;
    float _powerScaleStep;
    float _timeScale;
    float _timeScaleStep;
    volatile unsigned int _tickCount;
    volatile unsigned char *_timer_start_mask;
    volatile uint16_t *_timer_count;
    volatile unsigned char *_timer_interrupt_flag;
    volatile unsigned char *_timer_interrupt_mask_b;
    volatile unsigned char *_timer_interrupt_mask_a;
    unsigned int _timer_init;
    volatile bool _running;
    volatile pattern* _memQueue[2];
    volatile unsigned int _seq;

};

extern OSSex Toy;

#endif

